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


#include "moduldlg.hxx"
#include <basidesh.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <iderdll.hxx>
#include "iderdll2.hxx"
#include <svx/passwd.hxx>
#include <ucbhelper/content.hxx>
#include <rtl/uri.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/request.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svlbitm.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/weld.hxx>

#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/script/DocumentScriptLibraryContainer.hpp>
#include <com/sun/star/script/DocumentDialogLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/script/XLibraryContainerExport.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/packages/manifest/ManifestWriter.hpp>
#include <unotools/pathoptions.hxx>

#include <com/sun/star/util/VetoException.hpp>
#include <com/sun/star/script/ModuleSizeExceededRequest.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/implbase.hxx>

#include <cassert>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;

namespace
{

class DummyInteractionHandler  : public ::cppu::WeakImplHelper< task::XInteractionHandler >
{
    Reference< task::XInteractionHandler2 > m_xHandler;
public:
    explicit DummyInteractionHandler(const Reference<task::XInteractionHandler2>& xHandler)
        : m_xHandler(xHandler)
    {
    }

    virtual void SAL_CALL handle( const Reference< task::XInteractionRequest >& rRequest ) override
    {
        if ( m_xHandler.is() )
        {
        script::ModuleSizeExceededRequest aModSizeException;
        if ( rRequest->getRequest() >>= aModSizeException )
            m_xHandler->handle( rRequest );
        }
    }
};

// LibUserData
class LibUserData final
{
private:
    ScriptDocument m_aDocument;

public:
    explicit LibUserData(ScriptDocument const& rDocument)
        : m_aDocument(rDocument)
    {
    }

    const ScriptDocument& GetDocument() const { return m_aDocument; }
};

//  LibLBoxString
class LibLBoxString : public SvLBoxString
{
public:
    explicit LibLBoxString(const OUString& rTxt)
        : SvLBoxString(rTxt)
    {
    }

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
};

void LibLBoxString::Paint(const Point& rPos, SvTreeListBox& /*rDev*/, vcl::RenderContext& rRenderContext,
                          const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    // Change text color if library is read only:
    bool bReadOnly = false;
    if (rEntry.GetUserData())
    {
        ScriptDocument aDocument(static_cast<LibUserData*>(rEntry.GetUserData())->GetDocument());

        OUString aLibName = static_cast<const SvLBoxString&>(rEntry.GetItem(1)).GetText();
        Reference<script::XLibraryContainer2> xModLibContainer(aDocument.getLibraryContainer(E_SCRIPTS), UNO_QUERY);
        Reference<script::XLibraryContainer2 > xDlgLibContainer(aDocument.getLibraryContainer(E_DIALOGS), UNO_QUERY);
        bReadOnly = (xModLibContainer.is() && xModLibContainer->hasByName(aLibName) && xModLibContainer->isLibraryReadOnly(aLibName))
                 || (xDlgLibContainer.is() && xDlgLibContainer->hasByName(aLibName) && xDlgLibContainer->isLibraryReadOnly(aLibName));
    }
    if (bReadOnly)
        rRenderContext.DrawCtrlText(rPos, GetText(), 0, -1, DrawTextFlags::Disable);
    else
        rRenderContext.DrawText(rPos, GetText());
}

} // namespace

//  basctl::CheckBox
CheckBox::CheckBox(vcl::Window* pParent, WinBits nStyle)
    : SvTabListBox(pParent, nStyle)
    , eMode(ObjectMode::Module)
    , m_aDocument(ScriptDocument::getApplicationScriptDocument())
{
    long const aTabPositions[] = { 12 };  // TabPos needs at least one...
                                          // 12 because of the CheckBox
    SetTabs( SAL_N_ELEMENTS(aTabPositions), aTabPositions );
    Init();
}

VCL_BUILDER_FACTORY_CONSTRUCTOR(CheckBox, WB_TABSTOP)

CheckBox::~CheckBox()
{
    disposeOnce();
}

void CheckBox::dispose()
{
    pCheckButton.reset();

    // delete user data
    SvTreeListEntry* pEntry = First();
    while ( pEntry )
    {
        delete static_cast<LibUserData*>( pEntry->GetUserData() );
        pEntry->SetUserData( nullptr );
        pEntry = Next( pEntry );
    }
    SvTabListBox::dispose();
}

void CheckBox::Init()
{
    pCheckButton.reset(new SvLBoxButtonData(this));

    if (eMode == ObjectMode::Library)
        EnableCheckButton( pCheckButton.get() );
    else
        EnableCheckButton( nullptr );

    SetHighlightRange();
}

void CheckBox::SetMode (ObjectMode e)
{
    eMode = e;

    if (eMode == ObjectMode::Library)
        EnableCheckButton( pCheckButton.get() );
    else
        EnableCheckButton( nullptr );
}

SvTreeListEntry* CheckBox::DoInsertEntry( const OUString& rStr, sal_uLong nPos )
{
    return SvTabListBox::InsertEntryToColumn( rStr, nPos, 0 );
}

SvTreeListEntry* CheckBox::FindEntry( const OUString& rName )
{
    sal_uLong nCount = GetEntryCount();
    for ( sal_uLong i = 0; i < nCount; i++ )
    {
        SvTreeListEntry* pEntry = GetEntry( i );
        DBG_ASSERT( pEntry, "pEntry?!" );
        if ( rName.equalsIgnoreAsciiCase( GetEntryText( pEntry, 0 ) ) )
            return pEntry;
    }
    return nullptr;
}

void CheckBox::InitEntry(SvTreeListEntry* pEntry, const OUString& rTxt,
    const Image& rImg1, const Image& rImg2, SvLBoxButtonKind eButtonKind )
{
    SvTabListBox::InitEntry(pEntry, rTxt, rImg1, rImg2, eButtonKind);

    if (eMode == ObjectMode::Module)
    {
        // initialize all columns with own string class (column 0 == bitmap)
        sal_uInt16 nCount = pEntry->ItemCount();
        for ( sal_uInt16 nCol = 1; nCol < nCount; ++nCol )
        {
            SvLBoxString& rCol = static_cast<SvLBoxString&>(pEntry->GetItem( nCol ));
            pEntry->ReplaceItem(std::make_unique<LibLBoxString>( rCol.GetText() ), nCol);
        }
    }
}

bool CheckBox::EditingEntry( SvTreeListEntry* pEntry, Selection& )
{
    if (eMode != ObjectMode::Module)
        return false;

    DBG_ASSERT( pEntry, "No entry?" );

    // check, if Standard library
    OUString aLibName = GetEntryText( pEntry, 0 );
    if ( aLibName.equalsIgnoreAsciiCase( "Standard" ) )
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_CANNOTCHANGENAMESTDLIB)));
        xErrorBox->run();
        return false;
    }

    // check, if library is readonly
    Reference< script::XLibraryContainer2 > xModLibContainer( m_aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
    Reference< script::XLibraryContainer2 > xDlgLibContainer( m_aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
    if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) && !xModLibContainer->isLibraryLink( aLibName ) ) ||
         ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) && !xDlgLibContainer->isLibraryLink( aLibName ) ) )
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_LIBISREADONLY)));
        xErrorBox->run();
        return false;
    }

    // i24094: Password verification necessary for renaming
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && !xModLibContainer->isLibraryLoaded( aLibName ) )
    {
        bool bOK = true;
        // check password
        Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
        if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) && !xPasswd->isLibraryPasswordVerified( aLibName ) )
        {
            OUString aPassword;
            Reference< script::XLibraryContainer > xModLibContainer1( xModLibContainer, UNO_QUERY );
            bOK = QueryPassword( xModLibContainer1, aLibName, aPassword );
        }
        if ( !bOK )
            return false;
    }

    // TODO: check if library is reference/link

    return true;
}

bool CheckBox::EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewName )
{
    bool bValid = rNewName.getLength() <= 30 && IsValidSbxName(rNewName);
    OUString aOldName( GetEntryText( pEntry, 0 ) );
    if ( bValid && ( aOldName != rNewName ) )
    {
        try
        {
            Reference< script::XLibraryContainer2 > xModLibContainer( m_aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
            if ( xModLibContainer.is() )
                xModLibContainer->renameLibrary( aOldName, rNewName );

            Reference< script::XLibraryContainer2 > xDlgLibContainer( m_aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
            if ( xDlgLibContainer.is() )
                xDlgLibContainer->renameLibrary( aOldName, rNewName );

            MarkDocumentModified( m_aDocument );
            if (SfxBindings* pBindings = GetBindingsPtr())
            {
                pBindings->Invalidate( SID_BASICIDE_LIBSELECTOR );
                pBindings->Update( SID_BASICIDE_LIBSELECTOR );
            }
        }
        catch (const container::ElementExistException& )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                           VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_SBXNAMEALLREADYUSED)));
            xErrorBox->run();
            return false;
        }
        catch (const container::NoSuchElementException& )
        {
            DBG_UNHANDLED_EXCEPTION("basctl.basicide");
            return false;
        }
    }

    if ( !bValid )
    {
        OUString sWarning(rNewName.getLength() > 30 ? IDEResId(RID_STR_LIBNAMETOLONG) : IDEResId(RID_STR_BADSBXNAME));
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, sWarning));
        xErrorBox->run();

    }

    return bValid;
}

// NewObjectDialog
IMPL_LINK_NOARG(NewObjectDialog, OkButtonHandler, weld::Button&, void)
{
    if (!m_bCheckName || IsValidSbxName(m_xEdit->get_text()))
        m_xDialog->response(RET_OK);
    else
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_BADSBXNAME)));
        xErrorBox->run();
        m_xEdit->grab_focus();
    }
}

NewObjectDialog::NewObjectDialog(weld::Window * pParent, ObjectMode eMode, bool bCheckName)
    : GenericDialogController(pParent, "modules/BasicIDE/ui/newlibdialog.ui", "NewLibDialog")
    , m_xEdit(m_xBuilder->weld_entry("entry"))
    , m_xOKButton(m_xBuilder->weld_button("ok"))
    , m_bCheckName(bCheckName)
{
    switch (eMode)
    {
        case ObjectMode::Library:
            m_xDialog->set_title(IDEResId(RID_STR_NEWLIB));
            break;
        case ObjectMode::Module:
            m_xDialog->set_title(IDEResId(RID_STR_NEWMOD));
            break;
        case ObjectMode::Dialog:
            m_xDialog->set_title(IDEResId(RID_STR_NEWDLG));
            break;
        default:
            assert(false);
    }
    m_xOKButton->connect_clicked(LINK(this, NewObjectDialog, OkButtonHandler));
}

// GotoLineDialog
GotoLineDialog::GotoLineDialog(weld::Window* pParent )
    : GenericDialogController(pParent, "modules/BasicIDE/ui/gotolinedialog.ui", "GotoLineDialog")
    , m_xEdit(m_xBuilder->weld_entry("entry"))
    , m_xOKButton(m_xBuilder->weld_button("ok"))
{
    m_xEdit->grab_focus();
    m_xOKButton->connect_clicked(LINK(this, GotoLineDialog, OkButtonHandler));
}

GotoLineDialog::~GotoLineDialog()
{
}

sal_Int32 GotoLineDialog::GetLineNumber() const
{
    return m_xEdit->get_text().toInt32();
}

IMPL_LINK_NOARG(GotoLineDialog, OkButtonHandler, weld::Button&, void)
{
    if (GetLineNumber())
        m_xDialog->response(RET_OK);
    else
        m_xEdit->select_region(0, -1);
}

// ExportDialog
IMPL_LINK_NOARG(ExportDialog, OkButtonHandler, weld::Button&, void)
{
    m_bExportAsPackage = m_xExportAsPackageButton->get_active();
    m_xDialog->response(RET_OK);
}

ExportDialog::ExportDialog(weld::Window * pParent)
    : GenericDialogController(pParent, "modules/BasicIDE/ui/exportdialog.ui", "ExportDialog")
    , m_bExportAsPackage(false)
    , m_xExportAsPackageButton(m_xBuilder->weld_radio_button("extension"))
    , m_xOKButton(m_xBuilder->weld_button("ok"))
{
    m_xExportAsPackageButton->set_active(true);
    m_xOKButton->connect_clicked(LINK(this, ExportDialog, OkButtonHandler));
}

ExportDialog::~ExportDialog()
{
}

// LibPage
LibPage::LibPage(vcl::Window * pParent)
    : TabPage(pParent, "LibPage",
        "modules/BasicIDE/ui/libpage.ui")
    , m_aCurDocument(ScriptDocument::getApplicationScriptDocument())
    , m_eCurLocation(LIBRARY_LOCATION_UNKNOWN)
{
    get(m_pBasicsBox, "location");
    get(m_pLibBox, "library");
    Size aSize(m_pLibBox->LogicToPixel(Size(130, 87), MapMode(MapUnit::MapAppFont)));
    m_pLibBox->set_height_request(aSize.Height());
    m_pLibBox->set_width_request(aSize.Width());
    get(m_pEditButton, "edit");
    get(m_pPasswordButton, "password");
    get(m_pNewLibButton, "new");
    get(m_pInsertLibButton, "import");
    get(m_pExportButton, "export");
    get(m_pDelButton, "delete");

    pTabDlg = nullptr;

    m_pEditButton->SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    m_pNewLibButton->SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    m_pPasswordButton->SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    m_pExportButton->SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    m_pInsertLibButton->SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    m_pDelButton->SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    m_pLibBox->SetSelectHdl( LINK( this, LibPage, TreeListHighlightHdl ) );

    m_pBasicsBox->SetSelectHdl( LINK( this, LibPage, BasicSelectHdl ) );

    m_pLibBox->SetMode(ObjectMode::Module);
    m_pLibBox->EnableInplaceEditing(true);
    m_pLibBox->SetStyle( WB_HSCROLL | WB_BORDER | WB_TABSTOP );

    long const aTabPositions[] = { 30, 120 };
    m_pLibBox->SetTabs( SAL_N_ELEMENTS(aTabPositions), aTabPositions, MapUnit::MapPixel );

    FillListBox();
    m_pBasicsBox->SelectEntryPos( 0 );
    SetCurLib();

    CheckButtons();
}

LibPage::~LibPage()
{
    disposeOnce();
}

void LibPage::dispose()
{
    if (m_pBasicsBox)
    {
        const sal_Int32 nCount = m_pBasicsBox->GetEntryCount();
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            DocumentEntry* pEntry = static_cast<DocumentEntry*>(m_pBasicsBox->GetEntryData( i ));
            delete pEntry;
        }
    }
    m_pBasicsBox.clear();
    m_pLibBox.clear();
    m_pEditButton.clear();
    m_pPasswordButton.clear();
    m_pNewLibButton.clear();
    m_pInsertLibButton.clear();
    m_pExportButton.clear();
    m_pDelButton.clear();
    pTabDlg.clear();
    TabPage::dispose();
}

void LibPage::CheckButtons()
{
    SvTreeListEntry* pCur = m_pLibBox->GetCurEntry();
    if ( pCur )
    {
        OUString aLibName = SvTabListBox::GetEntryText( pCur, 0 );
        Reference< script::XLibraryContainer2 > xModLibContainer( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
        Reference< script::XLibraryContainer2 > xDlgLibContainer( m_aCurDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );

        if ( m_eCurLocation == LIBRARY_LOCATION_SHARE )
        {
            m_pPasswordButton->Disable();
            m_pNewLibButton->Disable();
            m_pInsertLibButton->Disable();
            m_pDelButton->Disable();
        }
        else if ( aLibName.equalsIgnoreAsciiCase( "Standard" ) )
        {
            m_pPasswordButton->Disable();
            m_pNewLibButton->Enable();
            m_pInsertLibButton->Enable();
            m_pExportButton->Disable();
            m_pDelButton->Disable();
        }
        else if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) ) ||
                  ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) ) )
        {
            m_pPasswordButton->Disable();
            m_pNewLibButton->Enable();
            m_pInsertLibButton->Enable();
            if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) && !xModLibContainer->isLibraryLink( aLibName ) ) ||
                 ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) && !xDlgLibContainer->isLibraryLink( aLibName ) ) )
                m_pDelButton->Disable();
            else
                m_pDelButton->Enable();
        }
        else
        {
            if ( xModLibContainer.is() && !xModLibContainer->hasByName( aLibName ) )
                m_pPasswordButton->Disable();
            else
                m_pPasswordButton->Enable();

            m_pNewLibButton->Enable();
            m_pInsertLibButton->Enable();
            m_pExportButton->Enable();
            m_pDelButton->Enable();
        }
    }
}

void LibPage::ActivatePage()
{
    SetCurLib();
}

void LibPage::DeactivatePage()
{
}

IMPL_LINK( LibPage, TreeListHighlightHdl, SvTreeListBox *, pBox, void )
{
    if ( pBox->IsSelected( pBox->GetHdlEntry() ) )
        CheckButtons();
}

IMPL_LINK_NOARG( LibPage, BasicSelectHdl, ListBox&, void )
{
    SetCurLib();
    CheckButtons();
}

IMPL_LINK( LibPage, ButtonHdl, Button *, pButton, void )
{
    if (pButton == m_pEditButton)
    {
        SfxAllItemSet aArgs( SfxGetpApp()->GetPool() );
        SfxRequest aRequest( SID_BASICIDE_APPEAR, SfxCallMode::SYNCHRON, aArgs );
        SfxGetpApp()->ExecuteSlot( aRequest );

        SfxUnoAnyItem aDocItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, Any( m_aCurDocument.getDocumentOrNull() ) );
        SvTreeListEntry* pCurEntry = m_pLibBox->GetCurEntry();
        DBG_ASSERT( pCurEntry, "Entry?!" );
        OUString aLibName( SvTabListBox::GetEntryText( pCurEntry, 0 ) );
        SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
        if (SfxDispatcher* pDispatcher = GetDispatcher())
            pDispatcher->ExecuteList( SID_BASICIDE_LIBSELECTED,
                SfxCallMode::ASYNCHRON, { &aDocItem, &aLibNameItem });
        EndTabDialog();
        return;
    }
    else if (pButton == m_pNewLibButton)
        NewLib();
    else if (pButton == m_pInsertLibButton)
        InsertLib();
    else if (pButton == m_pExportButton)
        Export();
    else if (pButton == m_pDelButton)
        DeleteCurrent();
    else if (pButton == m_pPasswordButton)
    {
        SvTreeListEntry* pCurEntry = m_pLibBox->GetCurEntry();
        OUString aLibName( SvTabListBox::GetEntryText( pCurEntry, 0 ) );

        // load module library (if not loaded)
        Reference< script::XLibraryContainer > xModLibContainer = m_aCurDocument.getLibraryContainer( E_SCRIPTS );
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && !xModLibContainer->isLibraryLoaded( aLibName ) )
        {
            Shell* pShell = GetShell();
            if (pShell)
                pShell->GetViewFrame()->GetWindow().EnterWait();
            xModLibContainer->loadLibrary( aLibName );
            if (pShell)
                pShell->GetViewFrame()->GetWindow().LeaveWait();
        }

        // load dialog library (if not loaded)
        Reference< script::XLibraryContainer > xDlgLibContainer = m_aCurDocument.getLibraryContainer( E_DIALOGS );
        if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && !xDlgLibContainer->isLibraryLoaded( aLibName ) )
        {
            Shell* pShell = GetShell();
            if (pShell)
                pShell->GetViewFrame()->GetWindow().EnterWait();
            xDlgLibContainer->loadLibrary( aLibName );
            if (pShell)
                pShell->GetViewFrame()->GetWindow().LeaveWait();
        }

        // check, if library is password protected
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) )
        {
            Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
            if ( xPasswd.is() )
            {
                bool const bProtected = xPasswd->isLibraryPasswordProtected( aLibName );

                // change password dialog
                SvxPasswordDialog aDlg(GetFrameWeld(), !bProtected);
                aDlg.SetCheckPasswordHdl(LINK(this, LibPage, CheckPasswordHdl));

                if (aDlg.run() == RET_OK)
                {
                    bool const bNewProtected = xPasswd->isLibraryPasswordProtected( aLibName );

                    if ( bNewProtected != bProtected )
                    {
                        sal_uLong nPos = m_pLibBox->GetModel()->GetAbsPos( pCurEntry );
                        m_pLibBox->GetModel()->Remove( pCurEntry );
                        ImpInsertLibEntry( aLibName, nPos );
                        m_pLibBox->SetCurEntry( m_pLibBox->GetEntry( nPos ) );
                    }

                    MarkDocumentModified( m_aCurDocument );
                }
            }
        }
    }
    CheckButtons();
}

IMPL_LINK( LibPage, CheckPasswordHdl, SvxPasswordDialog *, pDlg, bool )
{
    bool bRet = false;

    SvTreeListEntry* pCurEntry = m_pLibBox->GetCurEntry();
    OUString aLibName( SvTabListBox::GetEntryText( pCurEntry, 0 ) );
    Reference< script::XLibraryContainerPassword > xPasswd( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );

    if ( xPasswd.is() )
    {
        try
        {
            OUString aOldPassword( pDlg->GetOldPassword() );
            OUString aNewPassword( pDlg->GetNewPassword() );
            xPasswd->changeLibraryPassword( aLibName, aOldPassword, aNewPassword );
            bRet = true;
        }
        catch (...)
        {
        }
    }

    return bRet;
}

void LibPage::NewLib()
{
    createLibImpl(GetFrameWeld(), m_aCurDocument, m_pLibBox, static_cast<SbTreeListBox*>(nullptr));
}

void LibPage::InsertLib()
{
    Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    // file open dialog
    sfx2::FileDialogHelper aDlg(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE, pTabDlg ? pTabDlg->GetFrameWeld() : nullptr);
    const Reference <XFilePicker3>& xFP = aDlg.GetFilePicker();

    xFP->setTitle(IDEResId(RID_STR_APPENDLIBS));

    // filter
    OUString aTitle(IDEResId(RID_STR_BASIC));
    OUString aFilter;
    aFilter = "*.sbl;*.xlc;*.xlb"        // library files
              ";*.sdw;*.sxw;*.odt"       // text
              ";*.vor;*.stw;*.ott"       // text template
              ";*.sgl;*.sxg;*.odm"       // master document
              ";*.oth"                   // html document template
              ";*.sdc;*.sxc;*.ods"       // spreadsheet
              ";*.stc;*.ots"             // spreadsheet template
              ";*.sda;*.sxd;*.odg"       // drawing
              ";*.std;*.otg"             // drawing template
              ";*.sdd;*.sxi;*.odp"       // presentation
              ";*.sti;*.otp"             // presentation template
              ";*.sxm;*.odf";            // formula
    xFP->appendFilter( aTitle, aFilter );

    // set display directory and filter
    OUString aPath(GetExtraData()->GetAddLibPath());
    if ( !aPath.isEmpty() )
        xFP->setDisplayDirectory( aPath );
    else
    {
        // macro path from configuration management
        xFP->setDisplayDirectory( SvtPathOptions().GetWorkPath() );
    }

    OUString aLastFilter(GetExtraData()->GetAddLibFilter());
    if ( !aLastFilter.isEmpty() )
        xFP->setCurrentFilter( aLastFilter );
    else
        xFP->setCurrentFilter( IDEResId(RID_STR_BASIC) );

    if ( xFP->execute() != RET_OK )
            return;

    GetExtraData()->SetAddLibPath( xFP->getDisplayDirectory() );
    GetExtraData()->SetAddLibFilter( xFP->getCurrentFilter() );

    // library containers for import
    Reference< script::XLibraryContainer2 > xModLibContImport;
    Reference< script::XLibraryContainer2 > xDlgLibContImport;

    // file URLs
    Sequence< OUString > aFiles = xFP->getSelectedFiles();
    INetURLObject aURLObj( aFiles[0] );
    INetURLObject aModURLObj( aURLObj );
    INetURLObject aDlgURLObj( aURLObj );

    OUString aBase = aURLObj.getBase();
    OUString aModBase( "script" );
    OUString aDlgBase( "dialog" );

    if ( aBase == aModBase || aBase == aDlgBase )
    {
        aModURLObj.setBase( aModBase );
        aDlgURLObj.setBase( aDlgBase );
    }

    Reference< XSimpleFileAccess3 > xSFA( SimpleFileAccess::create(comphelper::getProcessComponentContext()) );

    OUString aModURL( aModURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    if ( xSFA->exists( aModURL ) )
    {
        xModLibContImport.set( script::DocumentScriptLibraryContainer::createWithURL(xContext, aModURL), UNO_QUERY );
    }

    OUString aDlgURL( aDlgURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    if ( xSFA->exists( aDlgURL ) )
    {
        xDlgLibContImport.set( script::DocumentDialogLibraryContainer::createWithURL(xContext, aDlgURL), UNO_QUERY );
    }

    if ( !xModLibContImport.is() && !xDlgLibContImport.is() )
        return;

    std::shared_ptr<LibDialog> xLibDlg;

    Reference< script::XLibraryContainer > xModLibContImp( xModLibContImport, UNO_QUERY );
    Reference< script::XLibraryContainer > xDlgLibContImp( xDlgLibContImport, UNO_QUERY );
    Sequence< OUString > aLibNames = GetMergedLibraryNames( xModLibContImp, xDlgLibContImp );
    sal_Int32 nLibCount = aLibNames.getLength();
    const OUString* pLibNames = aLibNames.getConstArray();
    for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
    {
        // library import dialog
        if (!xLibDlg)
        {
            xLibDlg.reset(new LibDialog(GetFrameWeld()));
            xLibDlg->SetStorageName( aURLObj.getName() );
        }

        // libbox entries
        OUString aLibName( pLibNames[ i ] );
        if ( !( ( xModLibContImport.is() && xModLibContImport->hasByName( aLibName ) && xModLibContImport->isLibraryLink( aLibName ) ) ||
                ( xDlgLibContImport.is() && xDlgLibContImport->hasByName( aLibName ) && xDlgLibContImport->isLibraryLink( aLibName ) ) ) )
        {
            weld::TreeView& rView = xLibDlg->GetLibBox();
            rView.append();
            const int nRow = rView.n_children() - 1;
            rView.set_toggle(nRow, true, 0);
            rView.set_text(nRow, aLibName, 1);
        }
    }

    if (!xLibDlg)
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_NOLIBINSTORAGE)));
        xErrorBox->run();
        return;
    }

    OUString aExtension( aURLObj.getExtension() );
    OUString aLibExtension( "xlb" );
    OUString aContExtension( "xlc" );

    // disable reference checkbox for documents and sbls
    if ( aExtension != aLibExtension && aExtension != aContExtension )
        xLibDlg->EnableReference(false);

    weld::DialogController::runAsync(xLibDlg, [aContExtension, aDlgURLObj, aExtension, aLibExtension, aModURLObj, xLibDlg, xDlgLibContImport, xModLibContImp, xModLibContImport, this](sal_Int32 nResult)
        {
            if (!nResult )
                return;

            bool bChanges = false;
            sal_uLong nNewPos = m_pLibBox->GetEntryCount();
            bool bRemove = false;
            bool bReplace = xLibDlg->IsReplace();
            bool bReference = xLibDlg->IsReference();
            weld::TreeView& rView = xLibDlg->GetLibBox();
            for (int nLib = 0, nChildren = rView.n_children(); nLib < nChildren; ++nLib)
            {
                if (rView.get_toggle(nLib, 0))
                {
                    OUString aLibName(rView.get_text(nLib, 1));
                    Reference< script::XLibraryContainer2 > xModLibContainer( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
                    Reference< script::XLibraryContainer2 > xDlgLibContainer( m_aCurDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );

                    // check, if the library is already existing
                    if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) ) ||
                         ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) ) )
                    {
                        if ( bReplace )
                        {
                            // check, if the library is the Standard library
                            if ( aLibName == "Standard" )
                            {
                                std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                               VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_REPLACESTDLIB)));
                                xErrorBox->run();
                                continue;
                            }

                            // check, if the library is readonly and not a link
                            if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) && !xModLibContainer->isLibraryLink( aLibName ) ) ||
                                 ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) && !xDlgLibContainer->isLibraryLink( aLibName ) ) )
                            {
                                OUString aErrStr( IDEResId(RID_STR_REPLACELIB) );
                                aErrStr = aErrStr.replaceAll("XX", aLibName) + "\n" + IDEResId(RID_STR_LIBISREADONLY);
                                std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                               VclMessageType::Warning, VclButtonsType::Ok, aErrStr));
                                xErrorBox->run();
                                continue;
                            }

                            // remove existing libraries
                            bRemove = true;
                        }
                        else
                        {
                            OUString aErrStr;
                            if ( bReference )
                                aErrStr = IDEResId(RID_STR_REFNOTPOSSIBLE);
                            else
                                aErrStr = IDEResId(RID_STR_IMPORTNOTPOSSIBLE);
                            aErrStr = aErrStr.replaceAll("XX", aLibName) + "\n" +IDEResId(RID_STR_SBXNAMEALLREADYUSED);
                            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                           VclMessageType::Warning, VclButtonsType::Ok, aErrStr));
                            xErrorBox->run();
                            continue;
                        }
                    }

                    // check, if the library is password protected
                    bool bOK = false;
                    OUString aPassword;
                    if ( xModLibContImport.is() && xModLibContImport->hasByName( aLibName ) )
                    {
                        Reference< script::XLibraryContainerPassword > xPasswd( xModLibContImport, UNO_QUERY );
                        if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) && !xPasswd->isLibraryPasswordVerified( aLibName ) && !bReference )
                        {
                            bOK = QueryPassword( xModLibContImp, aLibName, aPassword, true, true );

                            if ( !bOK )
                            {
                                OUString aErrStr( IDEResId(RID_STR_NOIMPORT) );
                                aErrStr = aErrStr.replaceAll("XX", aLibName);
                                std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                               VclMessageType::Warning, VclButtonsType::Ok, aErrStr));
                                xErrorBox->run();
                                continue;
                            }
                        }
                    }

                    // remove existing libraries
                    if ( bRemove )
                    {
                        // remove listbox entry
                        SvTreeListEntry* pEntry_ = m_pLibBox->FindEntry( aLibName );
                        if ( pEntry_ )
                            m_pLibBox->SvTreeListBox::GetModel()->Remove( pEntry_ );

                        // remove module library
                        if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) )
                            xModLibContainer->removeLibrary( aLibName );

                        // remove dialog library
                        if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) )
                            xDlgLibContainer->removeLibrary( aLibName );
                    }

                    // copy module library
                    if ( xModLibContImport.is() && xModLibContImport->hasByName( aLibName ) && xModLibContainer.is() && !xModLibContainer->hasByName( aLibName ) )
                    {
                        Reference< container::XNameContainer > xModLib;
                        if ( bReference )
                        {
                            // storage URL
                            INetURLObject aModStorageURLObj( aModURLObj );
                            if ( aExtension == aContExtension )
                            {
                                sal_Int32 nCount = aModStorageURLObj.getSegmentCount();
                                aModStorageURLObj.insertName( aLibName, false, nCount-1 );
                                aModStorageURLObj.setExtension( aLibExtension );
                                aModStorageURLObj.setFinalSlash();
                            }
                            OUString aModStorageURL( aModStorageURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

                            // create library link
                            xModLib.set( xModLibContainer->createLibraryLink( aLibName, aModStorageURL, true ), UNO_QUERY);
                        }
                        else
                        {
                            // create library
                            xModLib = xModLibContainer->createLibrary( aLibName );
                            if ( xModLib.is() )
                            {
                                // get import library
                                Reference< container::XNameContainer > xModLibImport;
                                Any aElement = xModLibContImport->getByName( aLibName );
                                aElement >>= xModLibImport;

                                if ( xModLibImport.is() )
                                {
                                    // load library
                                    if ( !xModLibContImport->isLibraryLoaded( aLibName ) )
                                        xModLibContImport->loadLibrary( aLibName );

                                    // copy all modules
                                    Sequence< OUString > aModNames = xModLibImport->getElementNames();
                                    sal_Int32 nModCount = aModNames.getLength();
                                    const OUString* pModNames = aModNames.getConstArray();
                                    for ( sal_Int32 i = 0 ; i < nModCount ; i++ )
                                    {
                                        OUString aModName( pModNames[ i ] );
                                        Any aElement_ = xModLibImport->getByName( aModName );
                                        xModLib->insertByName( aModName, aElement_ );
                                    }

                                    // set password
                                    if ( bOK )
                                    {
                                        Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                                        if ( xPasswd.is() )
                                        {
                                            try
                                            {
                                                xPasswd->changeLibraryPassword( aLibName, OUString(), aPassword );
                                            }
                                            catch (...)
                                            {
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // copy dialog library
                    if ( xDlgLibContImport.is() && xDlgLibContImport->hasByName( aLibName ) && xDlgLibContainer.is() && !xDlgLibContainer->hasByName( aLibName ) )
                    {
                        Reference< container::XNameContainer > xDlgLib;
                        if ( bReference )
                        {
                            // storage URL
                            INetURLObject aDlgStorageURLObj( aDlgURLObj );
                            if ( aExtension == aContExtension )
                            {
                                sal_Int32 nCount = aDlgStorageURLObj.getSegmentCount();
                                aDlgStorageURLObj.insertName( aLibName, false, nCount - 1 );
                                aDlgStorageURLObj.setExtension( aLibExtension );
                                aDlgStorageURLObj.setFinalSlash();
                            }
                            OUString aDlgStorageURL( aDlgStorageURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

                            // create library link
                            xDlgLib.set( xDlgLibContainer->createLibraryLink( aLibName, aDlgStorageURL, true ), UNO_QUERY);
                        }
                        else
                        {
                            // create library
                            xDlgLib = xDlgLibContainer->createLibrary( aLibName );
                            if ( xDlgLib.is() )
                            {
                                // get import library
                                Reference< container::XNameContainer > xDlgLibImport;
                                Any aElement = xDlgLibContImport->getByName( aLibName );
                                aElement >>= xDlgLibImport;

                                if ( xDlgLibImport.is() )
                                {
                                    // load library
                                    if ( !xDlgLibContImport->isLibraryLoaded( aLibName ) )
                                        xDlgLibContImport->loadLibrary( aLibName );

                                    // copy all dialogs
                                    Sequence< OUString > aDlgNames = xDlgLibImport->getElementNames();
                                    sal_Int32 nDlgCount = aDlgNames.getLength();
                                    const OUString* pDlgNames = aDlgNames.getConstArray();
                                    for ( sal_Int32 i = 0 ; i < nDlgCount ; i++ )
                                    {
                                        OUString aDlgName( pDlgNames[ i ] );
                                        Any aElement_ = xDlgLibImport->getByName( aDlgName );
                                        xDlgLib->insertByName( aDlgName, aElement_ );
                                    }
                                }
                            }
                        }
                    }

                    // insert listbox entry
                    ImpInsertLibEntry( aLibName, m_pLibBox->GetEntryCount() );
                    bChanges = true;
                }
            }

            SvTreeListEntry* pFirstNew = m_pLibBox->GetEntry( nNewPos );
            if ( pFirstNew )
                m_pLibBox->SetCurEntry( pFirstNew );

            if ( bChanges )
                MarkDocumentModified( m_aCurDocument );
        });
}

void LibPage::Export()
{
    SvTreeListEntry* pCurEntry = m_pLibBox->GetCurEntry();
    OUString aLibName( SvTabListBox::GetEntryText( pCurEntry, 0 ) );

    // Password verification
    Reference< script::XLibraryContainer2 > xModLibContainer( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );

    if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && !xModLibContainer->isLibraryLoaded( aLibName ) )
    {
        bool bOK = true;

        // check password
        Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
        if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) && !xPasswd->isLibraryPasswordVerified( aLibName ) )
        {
            OUString aPassword;
            Reference< script::XLibraryContainer > xModLibContainer1( xModLibContainer, UNO_QUERY );
            bOK = QueryPassword( xModLibContainer1, aLibName, aPassword );
        }
        if ( !bOK )
            return;
    }

    std::unique_ptr<ExportDialog> xNewDlg(new ExportDialog(GetFrameWeld()));
    if (xNewDlg->run() == RET_OK)
    {
        try
        {
            bool bExportAsPackage = xNewDlg->isExportAsPackage();
            //tdf#112063 ensure closing xNewDlg is not selected as
            //parent of file dialog from ExportAs...
            xNewDlg.reset();
            if (bExportAsPackage)
                ExportAsPackage( aLibName );
            else
                ExportAsBasic( aLibName );
        }
        catch(const util::VetoException& ) // user canceled operation
        {
        }
    }
}

void LibPage::implExportLib( const OUString& aLibName, const OUString& aTargetURL,
    const Reference< task::XInteractionHandler >& Handler )
{
    Reference< script::XLibraryContainerExport > xModLibContainerExport
        ( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
    Reference< script::XLibraryContainerExport > xDlgLibContainerExport
        ( m_aCurDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
    if ( xModLibContainerExport.is() )
        xModLibContainerExport->exportLibrary( aLibName, aTargetURL, Handler );

    if (!xDlgLibContainerExport.is())
        return;
    Reference<container::XNameAccess> xNameAcc(xDlgLibContainerExport, UNO_QUERY);
    if (!xNameAcc.is())
        return;
    if (!xNameAcc->hasByName(aLibName))
        return;
    xDlgLibContainerExport->exportLibrary(aLibName, aTargetURL, Handler);
}

// Implementation XCommandEnvironment

class OLibCommandEnvironment : public cppu::WeakImplHelper< XCommandEnvironment >
{
    Reference< task::XInteractionHandler > mxInteraction;

public:
    explicit OLibCommandEnvironment(const Reference<task::XInteractionHandler>& xInteraction)
        : mxInteraction( xInteraction )
    {}

    // Methods
    virtual Reference< task::XInteractionHandler > SAL_CALL getInteractionHandler() override;
    virtual Reference< XProgressHandler > SAL_CALL getProgressHandler() override;
};

Reference< task::XInteractionHandler > OLibCommandEnvironment::getInteractionHandler()
{
    return mxInteraction;
}

Reference< XProgressHandler > OLibCommandEnvironment::getProgressHandler()
{
    Reference< XProgressHandler > xRet;
    return xRet;
}

void LibPage::ExportAsPackage( const OUString& aLibName )
{
    // file open dialog
    sfx2::FileDialogHelper aDlg(ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, FileDialogFlags::NONE, pTabDlg ? pTabDlg->GetFrameWeld() : nullptr);
    const Reference <XFilePicker3>& xFP = aDlg.GetFilePicker();

    Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference< task::XInteractionHandler2 > xHandler( task::InteractionHandler::createWithParent(xContext, nullptr) );
    Reference< XSimpleFileAccess3 > xSFA = SimpleFileAccess::create(xContext);

    xFP->setTitle(IDEResId(RID_STR_EXPORTPACKAGE));

    // filter
    OUString aTitle(IDEResId(RID_STR_PACKAGE_BUNDLE));
    OUString aFilter;
    aFilter = "*.oxt" ;       // library files
    xFP->appendFilter( aTitle, aFilter );

    // set display directory and filter
    OUString aPath = GetExtraData()->GetAddLibPath();
    if ( !aPath.isEmpty() )
    {
        xFP->setDisplayDirectory( aPath );
    }
    else
    {
        // macro path from configuration management
        xFP->setDisplayDirectory( SvtPathOptions().GetWorkPath() );
    }
    xFP->setCurrentFilter( aTitle );

    if ( xFP->execute() == RET_OK )
    {
        GetExtraData()->SetAddLibPath(xFP->getDisplayDirectory());

        Sequence< OUString > aFiles = xFP->getSelectedFiles();
        INetURLObject aURL( aFiles[0] );
        if( aURL.getExtension().isEmpty() )
            aURL.setExtension( "oxt" );

        OUString aPackageURL( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

        OUString aTmpPath = SvtPathOptions().GetTempPath();
        INetURLObject aInetObj( aTmpPath );
        aInetObj.insertName( aLibName, true, INetURLObject::LAST_SEGMENT, INetURLObject::EncodeMechanism::All );
        OUString aSourcePath = aInetObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        if( xSFA->exists( aSourcePath ) )
            xSFA->kill( aSourcePath );
        Reference< task::XInteractionHandler > xDummyHandler( new DummyInteractionHandler( xHandler ) );
        implExportLib( aLibName, aTmpPath, xDummyHandler );

        Reference< XCommandEnvironment > xCmdEnv =
                new OLibCommandEnvironment(
                    Reference< task::XInteractionHandler >(
                        xHandler, UNO_QUERY));

        ::ucbhelper::Content sourceContent( aSourcePath, xCmdEnv, comphelper::getProcessComponentContext() );

        OUString destFolder = "vnd.sun.star.zip://" +
                              ::rtl::Uri::encode( aPackageURL,
                                                  rtl_UriCharClassRegName,
                                                  rtl_UriEncodeIgnoreEscapes,
                                                  RTL_TEXTENCODING_UTF8 ) +
                              "/";

        if( xSFA->exists( aPackageURL ) )
            xSFA->kill( aPackageURL );

        ::ucbhelper::Content destFolderContent( destFolder, xCmdEnv, comphelper::getProcessComponentContext() );
        destFolderContent.transferContent(
            sourceContent, ::ucbhelper::InsertOperation::Copy,
            OUString(), NameClash::OVERWRITE );

        INetURLObject aMetaInfInetObj( aTmpPath );
        aMetaInfInetObj.insertName( "META-INF",
            true, INetURLObject::LAST_SEGMENT, INetURLObject::EncodeMechanism::All );
        OUString aMetaInfFolder = aMetaInfInetObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        if( xSFA->exists( aMetaInfFolder ) )
            xSFA->kill( aMetaInfFolder );
        xSFA->createFolder( aMetaInfFolder );

        std::vector< Sequence<beans::PropertyValue> > manifest;
        const OUString strMediaType = "MediaType" ;
        const OUString strFullPath = "FullPath" ;
        const OUString strBasicMediaType = "application/vnd.sun.star.basic-library" ;

        OUString fullPath = aLibName
                          + "/" ;
        auto attribs(::comphelper::InitPropertySequence({
            { strFullPath, Any(fullPath) },
            { strMediaType, Any(strBasicMediaType) }
        }));
        manifest.push_back( attribs );

        // write into pipe:
        Reference<packages::manifest::XManifestWriter> xManifestWriter = packages::manifest::ManifestWriter::create( xContext );
        Reference<io::XOutputStream> xPipe( io::Pipe::create( xContext ), UNO_QUERY_THROW );
        xManifestWriter->writeManifestSequence(
            xPipe, Sequence< Sequence<beans::PropertyValue> >(
                &manifest[ 0 ], manifest.size() ) );

        aMetaInfInetObj.insertName( "manifest.xml",
            true, INetURLObject::LAST_SEGMENT, INetURLObject::EncodeMechanism::All );

        // write buffered pipe data to content:
        ::ucbhelper::Content manifestContent( aMetaInfInetObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xCmdEnv, comphelper::getProcessComponentContext() );
        manifestContent.writeStream( Reference<io::XInputStream>( xPipe, UNO_QUERY_THROW ), true );

        ::ucbhelper::Content MetaInfContent( aMetaInfFolder, xCmdEnv, comphelper::getProcessComponentContext() );
        destFolderContent.transferContent(
            MetaInfContent, ::ucbhelper::InsertOperation::Copy,
            OUString(), NameClash::OVERWRITE );

        if( xSFA->exists( aSourcePath ) )
            xSFA->kill( aSourcePath );
        if( xSFA->exists( aMetaInfFolder ) )
            xSFA->kill( aMetaInfFolder );
    }
}

void LibPage::ExportAsBasic( const OUString& aLibName )
{
    // Folder picker
    Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference< XFolderPicker2 > xFolderPicker = FolderPicker::create(xContext);
    Reference< task::XInteractionHandler2 > xHandler( task::InteractionHandler::createWithParent(xContext, nullptr) );

    xFolderPicker->setTitle(IDEResId(RID_STR_EXPORTBASIC));

    // set display directory and filter
    OUString aPath =GetExtraData()->GetAddLibPath();
    if( aPath.isEmpty() )
        aPath = SvtPathOptions().GetWorkPath();

    // INetURLObject aURL(m_sSavePath, INetProtocol::File);
    xFolderPicker->setDisplayDirectory( aPath );
    short nRet = xFolderPicker->execute();
    if( nRet == RET_OK )
    {
        OUString aTargetURL = xFolderPicker->getDirectory();
        GetExtraData()->SetAddLibPath(aTargetURL);

        Reference< task::XInteractionHandler > xDummyHandler( new DummyInteractionHandler( xHandler ) );
        implExportLib( aLibName, aTargetURL, xDummyHandler );
    }
}

void LibPage::DeleteCurrent()
{
    SvTreeListEntry* pCurEntry = m_pLibBox->GetCurEntry();
    OUString aLibName( SvTabListBox::GetEntryText( pCurEntry, 0 ) );

    // check, if library is link
    bool bIsLibraryLink = false;
    Reference< script::XLibraryContainer2 > xModLibContainer( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
    Reference< script::XLibraryContainer2 > xDlgLibContainer( m_aCurDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
    if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryLink( aLibName ) ) ||
         ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryLink( aLibName ) ) )
    {
        bIsLibraryLink = true;
    }

    if (QueryDelLib(aLibName, bIsLibraryLink, GetFrameWeld()))
    {
        // inform BasicIDE
        SfxUnoAnyItem aDocItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, Any( m_aCurDocument.getDocumentOrNull() ) );
        SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
        if (SfxDispatcher* pDispatcher = GetDispatcher())
            pDispatcher->ExecuteList(SID_BASICIDE_LIBREMOVED,
                      SfxCallMode::SYNCHRON, { &aDocItem, &aLibNameItem });

        // remove library from module and dialog library containers
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) )
            xModLibContainer->removeLibrary( aLibName );
        if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) )
            xDlgLibContainer->removeLibrary( aLibName );

        static_cast<SvTreeListBox&>(*m_pLibBox).GetModel()->Remove( pCurEntry );
        MarkDocumentModified( m_aCurDocument );
    }
}

void LibPage::EndTabDialog()
{
    DBG_ASSERT( pTabDlg, "TabDlg not set!" );
    if ( pTabDlg )
        pTabDlg->EndDialog( 1 );
}

void LibPage::FillListBox()
{
    InsertListBoxEntry( ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_USER );
    InsertListBoxEntry( ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_SHARE );

    ScriptDocuments aDocuments( ScriptDocument::getAllScriptDocuments( ScriptDocument::DocumentsSorted ) );
    for (auto const& doc : aDocuments)
    {
        InsertListBoxEntry( doc, LIBRARY_LOCATION_DOCUMENT );
    }
}

void LibPage::InsertListBoxEntry( const ScriptDocument& rDocument, LibraryLocation eLocation )
{
    OUString aEntryText( rDocument.getTitle( eLocation ) );
    const sal_Int32 nPos = m_pBasicsBox->InsertEntry( aEntryText );
    m_pBasicsBox->SetEntryData( nPos, new DocumentEntry(rDocument, eLocation) );
}

void LibPage::SetCurLib()
{
    const sal_Int32 nSelPos = m_pBasicsBox->GetSelectedEntryPos();
    DocumentEntry* pEntry = static_cast<DocumentEntry*>(m_pBasicsBox->GetEntryData( nSelPos ));
    if ( pEntry )
    {
        const ScriptDocument& aDocument( pEntry->GetDocument() );
        DBG_ASSERT( aDocument.isAlive(), "LibPage::SetCurLib: no document, or document is dead!" );
        if ( !aDocument.isAlive() )
            return;
        LibraryLocation eLocation = pEntry->GetLocation();
        if ( aDocument != m_aCurDocument || eLocation != m_eCurLocation )
        {
            m_aCurDocument = aDocument;
            m_eCurLocation = eLocation;
            m_pLibBox->SetDocument( aDocument );
            m_pLibBox->Clear();

            // get a sorted list of library names
            Sequence< OUString > aLibNames = aDocument.getLibraryNames();
            sal_Int32 nLibCount = aLibNames.getLength();
            const OUString* pLibNames = aLibNames.getConstArray();

            for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
            {
                OUString aLibName( pLibNames[ i ] );
                if ( eLocation == aDocument.getLibraryLocation( aLibName ) )
                    ImpInsertLibEntry( aLibName, i );
            }

            SvTreeListEntry* pEntry_ = m_pLibBox->FindEntry( "Standard" );
            if ( !pEntry_ )
                pEntry_ = m_pLibBox->GetEntry( 0 );
            m_pLibBox->SetCurEntry( pEntry_ );
        }
    }
}

SvTreeListEntry* LibPage::ImpInsertLibEntry( const OUString& rLibName, sal_uLong nPos )
{
    // check, if library is password protected
    bool bProtected = false;
    Reference< script::XLibraryContainer2 > xModLibContainer( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
    if ( xModLibContainer.is() && xModLibContainer->hasByName( rLibName ) )
    {
        Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
        if ( xPasswd.is() )
        {
            bProtected = xPasswd->isLibraryPasswordProtected( rLibName );
        }
    }

    SvTreeListEntry* pNewEntry = m_pLibBox->DoInsertEntry( rLibName, nPos );
    pNewEntry->SetUserData( new LibUserData(m_aCurDocument) );

    if (bProtected)
    {
        Image aImage(StockImage::Yes, RID_BMP_LOCKED);
        m_pLibBox->SetExpandedEntryBmp(pNewEntry, aImage);
        m_pLibBox->SetCollapsedEntryBmp(pNewEntry, aImage);
    }

    // check, if library is link
    if ( xModLibContainer.is() && xModLibContainer->hasByName( rLibName ) && xModLibContainer->isLibraryLink( rLibName ) )
    {
        OUString aLinkURL = xModLibContainer->getLibraryLinkURL( rLibName );
        m_pLibBox->SetEntryText( aLinkURL, pNewEntry, 1 );
    }

    return pNewEntry;
}

// Helper function
void createLibImpl(weld::Window* pWin, const ScriptDocument& rDocument,
                   CheckBox* pLibBox, TreeListBox* pBasicBox)
{
    OSL_ENSURE( rDocument.isAlive(), "createLibImpl: invalid document!" );
    if ( !rDocument.isAlive() )
        return;

    // create library name
    OUString aLibName;
    bool bValid = false;
    sal_Int32 i = 1;
    while ( !bValid )
    {
        aLibName = "Library" + OUString::number( i );
        if ( !rDocument.hasLibrary( E_SCRIPTS, aLibName ) && !rDocument.hasLibrary( E_DIALOGS, aLibName ) )
            bValid = true;
        i++;
    }

    NewObjectDialog aNewDlg(pWin, ObjectMode::Library);
    aNewDlg.SetObjectName(aLibName);

    if (aNewDlg.run())
    {
        if (!aNewDlg.GetObjectName().isEmpty())
            aLibName = aNewDlg.GetObjectName();

        if ( aLibName.getLength() > 30 )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(pWin,
                                                           VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_LIBNAMETOLONG)));
            xErrorBox->run();
        }
        else if ( !IsValidSbxName( aLibName ) )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(pWin,
                                                           VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_BADSBXNAME)));
            xErrorBox->run();
        }
        else if ( rDocument.hasLibrary( E_SCRIPTS, aLibName ) || rDocument.hasLibrary( E_DIALOGS, aLibName ) )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(pWin,
                                                           VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_SBXNAMEALLREADYUSED2)));
            xErrorBox->run();
        }
        else
        {
            try
            {
                // create module and dialog library
                Reference< container::XNameContainer > xModLib( rDocument.getOrCreateLibrary( E_SCRIPTS, aLibName ) );
                Reference< container::XNameContainer > xDlgLib( rDocument.getOrCreateLibrary( E_DIALOGS, aLibName ) );

                if( pLibBox )
                {
                    SvTreeListEntry* pEntry = pLibBox->DoInsertEntry( aLibName );
                    pEntry->SetUserData( new LibUserData( rDocument ) );
                    pLibBox->SetCurEntry( pEntry );
                }

                // create a module
                OUString aModName = rDocument.createObjectName( E_SCRIPTS, aLibName );
                OUString sModuleCode;
                if ( !rDocument.createModule( aLibName, aModName, true, sModuleCode ) )
                    throw Exception("could not create module " + aModName, nullptr);

                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, rDocument, aLibName, aModName, TYPE_MODULE );
                if (SfxDispatcher* pDispatcher = GetDispatcher())
                    pDispatcher->ExecuteList(SID_BASICIDE_SBXINSERTED,
                                          SfxCallMode::SYNCHRON, { &aSbxItem });

                if( pBasicBox )
                {
                    SvTreeListEntry* pEntry = pBasicBox->GetCurEntry();
                    SvTreeListEntry* pRootEntry = nullptr;
                    while( pEntry )
                    {
                        pRootEntry = pEntry;
                        pEntry = pBasicBox->GetParent( pEntry );
                    }

                    BrowseMode nMode = pBasicBox->GetMode();
                    bool bDlgMode = ( nMode & BrowseMode::Dialogs ) && !( nMode & BrowseMode::Modules );
                    const OUString sId = bDlgMode ? OUStringLiteral(RID_BMP_DLGLIB) : OUStringLiteral(RID_BMP_MODLIB);
                    SvTreeListEntry* pNewLibEntry = pBasicBox->AddEntry(
                        aLibName,
                        Image(StockImage::Yes, sId),
                        pRootEntry, false,
                        std::make_unique<Entry>(OBJ_TYPE_LIBRARY));
                    DBG_ASSERT( pNewLibEntry, "Insert entry failed!" );

                    if( pNewLibEntry )
                    {
                        SvTreeListEntry* pEntry_ = pBasicBox->AddEntry(
                            aModName,
                            Image(StockImage::Yes, RID_BMP_MODULE),
                            pNewLibEntry, false,
                            std::make_unique<Entry>(OBJ_TYPE_MODULE));
                        DBG_ASSERT( pEntry_, "Insert entry failed!" );
                        pBasicBox->SetCurEntry( pEntry_ );
                        pBasicBox->Select( pBasicBox->GetCurEntry() );      // OV-Bug?!
                    }
                }
            }
            catch (const uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("basctl.basicide");
            }
        }
    }
}

void createLibImpl(weld::Window* pWin, const ScriptDocument& rDocument,
                   CheckBox* pLibBox, SbTreeListBox* pBasicBox)
{
    OSL_ENSURE( rDocument.isAlive(), "createLibImpl: invalid document!" );
    if ( !rDocument.isAlive() )
        return;

    // create library name
    OUString aLibName;
    bool bValid = false;
    sal_Int32 i = 1;
    while ( !bValid )
    {
        aLibName = "Library" + OUString::number( i );
        if ( !rDocument.hasLibrary( E_SCRIPTS, aLibName ) && !rDocument.hasLibrary( E_DIALOGS, aLibName ) )
            bValid = true;
        i++;
    }

    NewObjectDialog aNewDlg(pWin, ObjectMode::Library);
    aNewDlg.SetObjectName(aLibName);

    if (aNewDlg.run())
    {
        if (!aNewDlg.GetObjectName().isEmpty())
            aLibName = aNewDlg.GetObjectName();

        if ( aLibName.getLength() > 30 )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(pWin,
                                                           VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_LIBNAMETOLONG)));
            xErrorBox->run();
        }
        else if ( !IsValidSbxName( aLibName ) )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(pWin,
                                                           VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_BADSBXNAME)));
            xErrorBox->run();
        }
        else if ( rDocument.hasLibrary( E_SCRIPTS, aLibName ) || rDocument.hasLibrary( E_DIALOGS, aLibName ) )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(pWin,
                                                           VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_SBXNAMEALLREADYUSED2)));
            xErrorBox->run();
        }
        else
        {
            try
            {
                // create module and dialog library
                Reference< container::XNameContainer > xModLib( rDocument.getOrCreateLibrary( E_SCRIPTS, aLibName ) );
                Reference< container::XNameContainer > xDlgLib( rDocument.getOrCreateLibrary( E_DIALOGS, aLibName ) );

                if( pLibBox )
                {
                    SvTreeListEntry* pEntry = pLibBox->DoInsertEntry( aLibName );
                    pEntry->SetUserData( new LibUserData( rDocument ) );
                    pLibBox->SetCurEntry( pEntry );
                }

                // create a module
                OUString aModName = rDocument.createObjectName( E_SCRIPTS, aLibName );
                OUString sModuleCode;
                if ( !rDocument.createModule( aLibName, aModName, true, sModuleCode ) )
                    throw Exception("could not create module " + aModName, nullptr);

                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, rDocument, aLibName, aModName, TYPE_MODULE );
                if (SfxDispatcher* pDispatcher = GetDispatcher())
                    pDispatcher->ExecuteList(SID_BASICIDE_SBXINSERTED,
                                          SfxCallMode::SYNCHRON, { &aSbxItem });

                if( pBasicBox )
                {
                    std::unique_ptr<weld::TreeIter> xIter(pBasicBox->make_iterator(nullptr));
                    bool bValidIter = pBasicBox->get_cursor(xIter.get());
                    std::unique_ptr<weld::TreeIter> xRootEntry(pBasicBox->make_iterator(xIter.get()));
                    while (bValidIter)
                    {
                        pBasicBox->copy_iterator(*xIter, *xRootEntry);
                        bValidIter = pBasicBox->iter_parent(*xIter);
                    }

                    BrowseMode nMode = pBasicBox->GetMode();
                    bool bDlgMode = ( nMode & BrowseMode::Dialogs ) && !( nMode & BrowseMode::Modules );
                    const OUString sId = bDlgMode ? OUStringLiteral(RID_BMP_DLGLIB) : OUStringLiteral(RID_BMP_MODLIB);
                    pBasicBox->AddEntry(aLibName, sId, xRootEntry.get(), false, std::make_unique<Entry>(OBJ_TYPE_LIBRARY));
                    pBasicBox->AddEntry(aModName, RID_BMP_MODULE, xRootEntry.get(), false, std::make_unique<Entry>(OBJ_TYPE_MODULE));
                    pBasicBox->set_cursor(*xRootEntry);
                    pBasicBox->select(*xRootEntry);
                }
            }
            catch (const uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("basctl.basicide");
            }
        }
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
