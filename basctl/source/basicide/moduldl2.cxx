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


#include <moduldlg.hxx>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <o3tl/make_unique.hxx>
#include <svx/passwd.hxx>
#include <ucbhelper/content.hxx>
#include <rtl/uri.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>
#include <vcl/builderfactory.hxx>

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
    long aTabs_[] = { 1, 12 };  // TabPos needs at least one...
                                // 12 because of the CheckBox
    SetTabs( aTabs_ );
    Init();
}

VCL_BUILDER_FACTORY_CONSTRUCTOR(CheckBox, WB_TABSTOP)

CheckBox::~CheckBox()
{
    disposeOnce();
}

void CheckBox::dispose()
{
    delete pCheckButton;
    pCheckButton = nullptr;

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
    pCheckButton = new SvLBoxButtonData(this);

    if (eMode == ObjectMode::Library)
        EnableCheckButton( pCheckButton );
    else
        EnableCheckButton( nullptr );

    SetHighlightRange();
}

void CheckBox::SetMode (ObjectMode::Mode e)
{
    eMode = e;

    if (eMode == ObjectMode::Library)
        EnableCheckButton( pCheckButton );
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

void CheckBox::CheckEntryPos( sal_uLong nPos )
{
    if ( nPos < GetEntryCount() )
    {
        SvTreeListEntry* pEntry = GetEntry( nPos );

        if ( GetCheckButtonState( pEntry ) != SvButtonState::Checked )
            SetCheckButtonState( pEntry, SvButtonState::Checked );
    }
}

bool CheckBox::IsChecked( sal_uLong nPos ) const
{
    if ( nPos < GetEntryCount() )
        return GetCheckButtonState(GetEntry(nPos)) == SvButtonState::Checked;
    return false;
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
            pEntry->ReplaceItem(o3tl::make_unique<LibLBoxString>( rCol.GetText() ), nCol);
        }
    }
}

bool CheckBox::EditingEntry( SvTreeListEntry* pEntry, Selection& )
{
    if (eMode != ObjectMode::Module)
        return false;

    DBG_ASSERT( pEntry, "Kein Eintrag?" );

    // check, if Standard library
    OUString aLibName = GetEntryText( pEntry, 0 );
    if ( aLibName.equalsIgnoreAsciiCase( "Standard" ) )
    {
        ScopedVclPtrInstance<MessageDialog>(this, IDE_RESSTR(RID_STR_CANNOTCHANGENAMESTDLIB))->Execute();
        return false;
    }

    // check, if library is readonly
    Reference< script::XLibraryContainer2 > xModLibContainer( m_aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
    Reference< script::XLibraryContainer2 > xDlgLibContainer( m_aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
    if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) && !xModLibContainer->isLibraryLink( aLibName ) ) ||
         ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) && !xDlgLibContainer->isLibraryLink( aLibName ) ) )
    {
        ScopedVclPtrInstance<MessageDialog>(this, IDE_RESSTR(RID_STR_LIBISREADONLY))->Execute();
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
            ScopedVclPtrInstance<MessageDialog>(this, IDE_RESSTR(RID_STR_SBXNAMEALLREADYUSED))->Execute();
            return false;
        }
        catch (const container::NoSuchElementException& )
        {
            DBG_UNHANDLED_EXCEPTION();
            return false;
        }
    }

    if ( !bValid )
    {
        if ( rNewName.getLength() > 30 )
            ScopedVclPtrInstance<MessageDialog>(this, IDE_RESSTR(RID_STR_LIBNAMETOLONG))->Execute();
        else
            ScopedVclPtrInstance<MessageDialog>(this, IDE_RESSTR(RID_STR_BADSBXNAME))->Execute();
    }

    return bValid;
}

// NewObjectDialog
IMPL_LINK_NOARG(NewObjectDialog, OkButtonHandler, Button*, void)
{
    if (IsValidSbxName(m_pEdit->GetText()))
        EndDialog(1);
    else
    {
        ScopedVclPtrInstance<MessageDialog>(this, IDE_RESSTR(RID_STR_BADSBXNAME))->Execute();
        m_pEdit->GrabFocus();
    }
}

NewObjectDialog::NewObjectDialog(vcl::Window * pParent, ObjectMode::Mode eMode,
    bool bCheckName)
    : ModalDialog(pParent, "NewLibDialog", "modules/BasicIDE/ui/newlibdialog.ui")
{
    get(m_pOKButton, "ok");
    get(m_pEdit, "entry");

    m_pEdit->GrabFocus();

    switch (eMode)
    {
        case ObjectMode::Library:
            SetText( IDE_RESSTR(RID_STR_NEWLIB) );
            break;
        case ObjectMode::Module:
            SetText( IDE_RESSTR(RID_STR_NEWMOD) );
            break;
        case ObjectMode::Method:
            SetText( IDE_RESSTR(RID_STR_NEWMETH) );
            break;
        case ObjectMode::Dialog:
            SetText( IDE_RESSTR(RID_STR_NEWDLG) );
            break;
        default:
            assert(false);
    }

    if (bCheckName)
        m_pOKButton->SetClickHdl(LINK(this, NewObjectDialog, OkButtonHandler));
}

NewObjectDialog::~NewObjectDialog()
{
    disposeOnce();
}

void NewObjectDialog::dispose()
{
    m_pEdit.clear();
    m_pOKButton.clear();
    ModalDialog::dispose();
}

// GotoLineDialog
GotoLineDialog::GotoLineDialog(vcl::Window * pParent )
    : ModalDialog(pParent, "GotoLineDialog",
        "modules/BasicIDE/ui/gotolinedialog.ui")
{
    get(m_pEdit, "entry");
    get(m_pOKButton, "ok");
    m_pEdit->GrabFocus();
    m_pOKButton->SetClickHdl(LINK(this, GotoLineDialog, OkButtonHandler));
}

GotoLineDialog::~GotoLineDialog()
{
    disposeOnce();
}

void GotoLineDialog::dispose()
{
    m_pEdit.clear();
    m_pOKButton.clear();
    ModalDialog::dispose();
}

sal_Int32 GotoLineDialog::GetLineNumber() const
{
    return m_pEdit->GetText().toInt32();
}

IMPL_LINK_NOARG(GotoLineDialog, OkButtonHandler, Button*, void)
{
    if ( GetLineNumber() )
        EndDialog(1);
    else
        m_pEdit->SetText(m_pEdit->GetText(), Selection(0, m_pEdit->GetText().getLength()));
}

// ExportDialog
IMPL_LINK_NOARG(ExportDialog, OkButtonHandler, Button*, void)
{
    mbExportAsPackage = m_pExportAsPackageButton->IsChecked();
    EndDialog(1);
}

ExportDialog::ExportDialog(vcl::Window * pParent)
    : ModalDialog(pParent, "ExportDialog",
        "modules/BasicIDE/ui/exportdialog.ui")
    , mbExportAsPackage(false)
{
    get(m_pExportAsPackageButton, "extension");
    get(m_pOKButton, "ok");

    m_pExportAsPackageButton->Check();
    m_pOKButton->SetClickHdl(LINK(this, ExportDialog, OkButtonHandler));
}

ExportDialog::~ExportDialog()
{
    disposeOnce();
}

void ExportDialog::dispose()
{
    m_pExportAsPackageButton.clear();
    m_pOKButton.clear();
    ModalDialog::dispose();
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
    Size aSize(m_pLibBox->LogicToPixel(Size(130, 87), MapUnit::MapAppFont));
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

    long aTabs[] = { 2, 30, 120 };
    m_pLibBox->SetTabs( aTabs, MapUnit::MapPixel );

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

        SfxUsrAnyItem aDocItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, Any( m_aCurDocument.getDocumentOrNull() ) );
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
                VclPtrInstance< SvxPasswordDialog > pDlg( this, true, !bProtected );
                pDlg->SetCheckPasswordHdl( LINK( this, LibPage, CheckPasswordHdl ) );

                if ( pDlg->Execute() == RET_OK )
                {
                    bool const bNewProtected = xPasswd->isLibraryPasswordProtected( aLibName );

                    if ( bNewProtected != bProtected )
                    {
                        sal_uLong nPos = (sal_uLong)m_pLibBox->GetModel()->GetAbsPos( pCurEntry );
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
    createLibImpl( static_cast<vcl::Window*>( this ), m_aCurDocument, m_pLibBox, nullptr);
}

void LibPage::InsertLib()
{
    Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    // file open dialog
    Reference < XFilePicker3 > xFP = FilePicker::createWithMode(xContext, TemplateDescription::FILEOPEN_SIMPLE);
    xFP->setTitle(IDEResId(RID_STR_APPENDLIBS).toString());

    // filter
    OUString aTitle(IDEResId(RID_STR_BASIC).toString());
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
        xFP->setCurrentFilter( IDE_RESSTR(RID_STR_BASIC) );

    if ( xFP->execute() == RET_OK )
    {
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

        if ( xModLibContImport.is() || xDlgLibContImport.is() )
        {
            VclPtr<LibDialog> pLibDlg;

            Reference< script::XLibraryContainer > xModLibContImp( xModLibContImport, UNO_QUERY );
            Reference< script::XLibraryContainer > xDlgLibContImp( xDlgLibContImport, UNO_QUERY );
            Sequence< OUString > aLibNames = GetMergedLibraryNames( xModLibContImp, xDlgLibContImp );
            sal_Int32 nLibCount = aLibNames.getLength();
            const OUString* pLibNames = aLibNames.getConstArray();
            for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
            {
                // library import dialog
                if ( !pLibDlg )
                {
                    pLibDlg.reset(VclPtr<LibDialog>::Create( this ));
                    pLibDlg->SetStorageName( aURLObj.getName() );
                    pLibDlg->GetLibBox().SetMode(ObjectMode::Library);
                }

                // libbox entries
                OUString aLibName( pLibNames[ i ] );
                if ( !( ( xModLibContImport.is() && xModLibContImport->hasByName( aLibName ) && xModLibContImport->isLibraryLink( aLibName ) ) ||
                        ( xDlgLibContImport.is() && xDlgLibContImport->hasByName( aLibName ) && xDlgLibContImport->isLibraryLink( aLibName ) ) ) )
                {
                    SvTreeListEntry* pEntry = pLibDlg->GetLibBox().DoInsertEntry( aLibName );
                    sal_uInt16 nPos = (sal_uInt16) pLibDlg->GetLibBox().GetModel()->GetAbsPos( pEntry );
                    pLibDlg->GetLibBox().CheckEntryPos(nPos);
                }
            }

            if ( !pLibDlg )
                ScopedVclPtrInstance<MessageDialog>(this, IDE_RESSTR(RID_STR_NOLIBINSTORAGE), VclMessageType::Info)->Execute();
            else
            {
                bool bChanges = false;
                OUString aExtension( aURLObj.getExtension() );
                OUString aLibExtension( "xlb" );
                OUString aContExtension( "xlc" );

                // disable reference checkbox for documents and sbls
                if ( aExtension != aLibExtension && aExtension != aContExtension )
                    pLibDlg->EnableReference(false);

                if ( pLibDlg->Execute() )
                {
                    sal_uLong nNewPos = m_pLibBox->GetEntryCount();
                    bool bRemove = false;
                    bool bReplace = pLibDlg->IsReplace();
                    bool bReference = pLibDlg->IsReference();
                    for ( sal_uLong nLib = 0; nLib < pLibDlg->GetLibBox().GetEntryCount(); nLib++ )
                    {
                        if ( pLibDlg->GetLibBox().IsChecked( nLib ) )
                        {
                            SvTreeListEntry* pEntry = pLibDlg->GetLibBox().GetEntry( nLib );
                            DBG_ASSERT( pEntry, "Entry?!" );
                            OUString aLibName( SvTabListBox::GetEntryText( pEntry, 0 ) );
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
                                        ScopedVclPtrInstance<MessageDialog>(this, IDE_RESSTR(RID_STR_REPLACESTDLIB))->Execute();
                                        continue;
                                    }

                                    // check, if the library is readonly and not a link
                                    if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) && !xModLibContainer->isLibraryLink( aLibName ) ) ||
                                         ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) && !xDlgLibContainer->isLibraryLink( aLibName ) ) )
                                    {
                                        OUString aErrStr( IDE_RESSTR(RID_STR_REPLACELIB) );
                                        aErrStr = aErrStr.replaceAll("XX", aLibName) + "\n" + IDE_RESSTR(RID_STR_LIBISREADONLY);
                                        ScopedVclPtrInstance<MessageDialog>(this, aErrStr)->Execute();
                                        continue;
                                    }

                                    // remove existing libraries
                                    bRemove = true;
                                }
                                else
                                {
                                    OUString aErrStr;
                                    if ( bReference )
                                        aErrStr = IDE_RESSTR(RID_STR_REFNOTPOSSIBLE);
                                    else
                                        aErrStr = IDE_RESSTR(RID_STR_IMPORTNOTPOSSIBLE);
                                    aErrStr = aErrStr.replaceAll("XX", aLibName) + "\n" +IDE_RESSTR(RID_STR_SBXNAMEALLREADYUSED);
                                    ScopedVclPtrInstance<MessageDialog>(this, aErrStr)->Execute();
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
                                        OUString aErrStr( IDE_RESSTR(RID_STR_NOIMPORT) );
                                        aErrStr = aErrStr.replaceAll("XX", aLibName);
                                        ScopedVclPtrInstance<MessageDialog>(this, aErrStr)->Execute();
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
                }

                pLibDlg.reset();
                if ( bChanges )
                    MarkDocumentModified( m_aCurDocument );
            }
        }
    }
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

    ScopedVclPtrInstance< ExportDialog > aNewDlg(this);
    if (aNewDlg->Execute() == RET_OK)
    {
        try
        {
            if (aNewDlg->isExportAsPackage())
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
    Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference< task::XInteractionHandler2 > xHandler( task::InteractionHandler::createWithParent(xContext, nullptr) );
    Reference< XSimpleFileAccess3 > xSFA = SimpleFileAccess::create(xContext);

    Reference < XFilePicker3 > xFP = FilePicker::createWithMode(xContext, TemplateDescription::FILESAVE_SIMPLE);

    xFP->setTitle(IDEResId(RID_STR_EXPORTPACKAGE).toString());

    // filter
    OUString aTitle(IDEResId(RID_STR_PACKAGE_BUNDLE).toString());
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
            static_cast<XCommandEnvironment*>(
                new OLibCommandEnvironment(
                    Reference< task::XInteractionHandler >(
                        xHandler, UNO_QUERY ) ) );

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

        ::std::vector< Sequence<beans::PropertyValue> > manifest;
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

    xFolderPicker->setTitle(IDEResId(RID_STR_EXPORTBASIC).toString());

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

    if ( QueryDelLib( aLibName, bIsLibraryLink, this ) )
    {
        // inform BasicIDE
        SfxUsrAnyItem aDocItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, Any( m_aCurDocument.getDocumentOrNull() ) );
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
    DBG_ASSERT( pTabDlg, "TabDlg nicht gesetzt!" );
    if ( pTabDlg )
        pTabDlg->EndDialog( 1 );
}

void LibPage::FillListBox()
{
    InsertListBoxEntry( ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_USER );
    InsertListBoxEntry( ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_SHARE );

    ScriptDocuments aDocuments( ScriptDocument::getAllScriptDocuments( ScriptDocument::DocumentsSorted ) );
    for (   ScriptDocuments::const_iterator doc = aDocuments.begin();
            doc != aDocuments.end();
            ++doc
        )
    {
        InsertListBoxEntry( *doc, LIBRARY_LOCATION_DOCUMENT );
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
    const sal_Int32 nSelPos = m_pBasicsBox->GetSelectEntryPos();
    DocumentEntry* pEntry = static_cast<DocumentEntry*>(m_pBasicsBox->GetEntryData( nSelPos ));
    if ( pEntry )
    {
        ScriptDocument aDocument( pEntry->GetDocument() );
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
        Image aImage(BitmapEx(IDEResId(RID_BMP_LOCKED)));
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
void createLibImpl( vcl::Window* pWin, const ScriptDocument& rDocument,
                    CheckBox* pLibBox, TreeListBox* pBasicBox )
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

    ScopedVclPtrInstance< NewObjectDialog > aNewDlg(pWin, ObjectMode::Library);
    aNewDlg->SetObjectName(aLibName);

    if (aNewDlg->Execute())
    {
        if (!aNewDlg->GetObjectName().isEmpty())
            aLibName = aNewDlg->GetObjectName();

        if ( aLibName.getLength() > 30 )
        {
            ScopedVclPtrInstance<MessageDialog>(pWin, IDEResId(RID_STR_LIBNAMETOLONG).toString())->Execute();
        }
        else if ( !IsValidSbxName( aLibName ) )
        {
            ScopedVclPtrInstance<MessageDialog>(pWin, IDEResId(RID_STR_BADSBXNAME).toString())->Execute();
        }
        else if ( rDocument.hasLibrary( E_SCRIPTS, aLibName ) || rDocument.hasLibrary( E_DIALOGS, aLibName ) )
        {
            ScopedVclPtrInstance<MessageDialog>(pWin, IDEResId(RID_STR_SBXNAMEALLREADYUSED2).toString())->Execute();
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
                    throw Exception();

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

                    sal_uInt16 nMode = pBasicBox->GetMode();
                    bool bDlgMode = ( nMode & BROWSEMODE_DIALOGS ) && !( nMode & BROWSEMODE_MODULES );
                    const sal_uInt16 nId = bDlgMode ? RID_BMP_DLGLIB : RID_BMP_MODLIB;
                    SvTreeListEntry* pNewLibEntry = pBasicBox->AddEntry(
                        aLibName,
                        Image(BitmapEx(IDEResId(nId))),
                        pRootEntry, false,
                        o3tl::make_unique<Entry>(OBJ_TYPE_LIBRARY));
                    DBG_ASSERT( pNewLibEntry, "InsertEntry fehlgeschlagen!" );

                    if( pNewLibEntry )
                    {
                        SvTreeListEntry* pEntry_ = pBasicBox->AddEntry(
                            aModName,
                            Image(BitmapEx(IDEResId(RID_BMP_MODULE))),
                            pNewLibEntry, false,
                            o3tl::make_unique<Entry>(OBJ_TYPE_MODULE));
                        DBG_ASSERT( pEntry_, "InsertEntry fehlgeschlagen!" );
                        pBasicBox->SetCurEntry( pEntry_ );
                        pBasicBox->Select( pBasicBox->GetCurEntry() );      // OV-Bug?!
                    }
                }
            }
            catch (const uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }
}
} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
