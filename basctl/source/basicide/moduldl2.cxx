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

#include <vcl/msgbox.hxx>
#include <sot/storinfo.hxx>

#include <moduldlg.hrc>
#include <moduldlg.hxx>
#include <basidesh.hrc>
#include <basidesh.hxx>
#include <bastypes.hxx>
#include <basobj.hxx>
#include <baside2.hrc>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <svx/passwd.hxx>
#include <sbxitem.hxx>
#include <basdoc.hxx>
#include <ucbhelper/content.hxx>
#include <rtl/uri.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>

#include <sot/storage.hxx>
#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
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
#include <comphelper/processfactory.hxx>

#include <com/sun/star/util/VetoException.hpp>
#include <com/sun/star/script/ModuleSizeExceededRequest.hpp>

#include <cassert>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;


typedef ::cppu::WeakImplHelper1< task::XInteractionHandler > HandlerImpl_BASE;

namespace
{

class DummyInteractionHandler  : public HandlerImpl_BASE
{
    Reference< task::XInteractionHandler2 > m_xHandler;
public:
    DummyInteractionHandler( const Reference< task::XInteractionHandler2 >& xHandler ) : m_xHandler( xHandler ){}

    virtual void SAL_CALL handle( const Reference< task::XInteractionRequest >& rRequest ) throw (::com::sun::star::uno::RuntimeException)
    {
        if ( m_xHandler.is() )
        {
        script::ModuleSizeExceededRequest aModSizeException;
        if ( rRequest->getRequest() >>= aModSizeException )
            m_xHandler->handle( rRequest );
        }
    }
};

//----------------------------------------------------------------------------
//  LibUserData
//----------------------------------------------------------------------------
class LibUserData
{
private:
    ScriptDocument m_aDocument;

public:
    LibUserData (ScriptDocument const& rDocument) : m_aDocument(rDocument) { }
    virtual ~LibUserData() {};

    const ScriptDocument& GetDocument() const { return m_aDocument; }
};

//----------------------------------------------------------------------------
//  LibLBoxString
//----------------------------------------------------------------------------

class LibLBoxString : public SvLBoxString
{
public:
    LibLBoxString( SvTreeListEntry* pEntry, sal_uInt16 nFlags, const OUString& rTxt ) :
        SvLBoxString( pEntry, nFlags, rTxt ) {}

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry);
};

//----------------------------------------------------------------------------

void LibLBoxString::Paint(const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* /*pView*/, const SvTreeListEntry* pEntry)
{
    // Change text color if library is read only:
    bool bReadOnly = false;
    if (pEntry && pEntry->GetUserData())
    {
        ScriptDocument aDocument(
            static_cast<LibUserData*>(pEntry->GetUserData())->
            GetDocument() );

        OUString aLibName = static_cast<const SvLBoxString*>(pEntry->GetItem(1))->GetText();
        Reference< script::XLibraryContainer2 > xModLibContainer(
            aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY);
        Reference< script::XLibraryContainer2 > xDlgLibContainer(
            aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY);
        bReadOnly
            = (xModLibContainer.is() && xModLibContainer->hasByName(aLibName)
               && xModLibContainer->isLibraryReadOnly(aLibName))
            || (xDlgLibContainer.is() && xDlgLibContainer->hasByName(aLibName)
                && xDlgLibContainer->isLibraryReadOnly(aLibName));
    }
    if (bReadOnly)
        rDev.DrawCtrlText(rPos, GetText(), 0, -1, TEXT_DRAW_DISABLE);
    else
        rDev.DrawText(rPos, GetText());
}

} // namespace


//----------------------------------------------------------------------------
//  basctl::CheckBox
//----------------------------------------------------------------------------

CheckBox::CheckBox( Window* pParent, const ResId& rResId )
    :SvTabListBox( pParent, rResId )
    ,eMode(ObjectMode::Module)
    ,m_aDocument( ScriptDocument::getApplicationScriptDocument() )
{
    long aTabs_[] = { 1, 12 };  // TabPos needs at least one...
                                // 12 because of the CheckBox
    SetTabs( aTabs_ );
    Init();
}

//----------------------------------------------------------------------------

CheckBox::~CheckBox()
{
    delete pCheckButton;

    // delete user data
    SvTreeListEntry* pEntry = First();
    while ( pEntry )
    {
        delete static_cast<LibUserData*>(pEntry->GetUserData());
        pEntry = Next( pEntry );
    }
}

//----------------------------------------------------------------------------

void CheckBox::Init()
{
    pCheckButton = new SvLBoxButtonData(this);

    if (eMode == ObjectMode::Library)
        EnableCheckButton( pCheckButton );
    else
        EnableCheckButton( 0 );

    SetHighlightRange();
}

//----------------------------------------------------------------------------

void CheckBox::SetMode (ObjectMode::Mode e)
{
    eMode = e;

    if (eMode == ObjectMode::Library)
        EnableCheckButton( pCheckButton );
    else
        EnableCheckButton( 0 );
}

//----------------------------------------------------------------------------

SvTreeListEntry* CheckBox::DoInsertEntry( const OUString& rStr, sal_uLong nPos )
{
    return SvTabListBox::InsertEntryToColumn( rStr, nPos, 0 );
}

//----------------------------------------------------------------------------

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
    return 0;
}

//----------------------------------------------------------------------------

void CheckBox::CheckEntryPos( sal_uLong nPos )
{
    if ( nPos < GetEntryCount() )
    {
        SvTreeListEntry* pEntry = GetEntry( nPos );

        if ( GetCheckButtonState( pEntry ) != SV_BUTTON_CHECKED )
            SetCheckButtonState( pEntry, SvButtonState(SV_BUTTON_CHECKED) );
    }
}

//----------------------------------------------------------------------------

bool CheckBox::IsChecked( sal_uLong nPos ) const
{
    if ( nPos < GetEntryCount() )
        return GetCheckButtonState(GetEntry(nPos)) == SV_BUTTON_CHECKED;
    return false;
}

//----------------------------------------------------------------------------

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
            SvLBoxString* pCol = (SvLBoxString*)pEntry->GetItem( nCol );
            LibLBoxString* pStr = new LibLBoxString( pEntry, 0, pCol->GetText() );
            pEntry->ReplaceItem( pStr, nCol );
        }
    }
}

//----------------------------------------------------------------------------

sal_Bool CheckBox::EditingEntry( SvTreeListEntry* pEntry, Selection& )
{
    if (eMode != ObjectMode::Module)
        return false;

    DBG_ASSERT( pEntry, "Kein Eintrag?" );

    // check, if Standard library
    OUString aLibName = GetEntryText( pEntry, 0 );
    if ( aLibName.equalsIgnoreAsciiCase( "Standard" ) )
    {
        ErrorBox( this, WB_OK | WB_DEF_OK, IDE_RESSTR(RID_STR_CANNOTCHANGENAMESTDLIB) ).Execute();
        return false;
    }

    // check, if library is readonly
    Reference< script::XLibraryContainer2 > xModLibContainer( m_aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
    Reference< script::XLibraryContainer2 > xDlgLibContainer( m_aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
    if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) && !xModLibContainer->isLibraryLink( aLibName ) ) ||
         ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) && !xDlgLibContainer->isLibraryLink( aLibName ) ) )
    {
        ErrorBox( this, WB_OK | WB_DEF_OK, IDE_RESSTR(RID_STR_LIBISREADONLY) ).Execute();
        return false;
    }

    // i24094: Password verification necessary for renaming
    bool bOK = true;
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && !xModLibContainer->isLibraryLoaded( aLibName ) )
    {
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

//----------------------------------------------------------------------------

sal_Bool CheckBox::EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewName )
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
            ErrorBox( this, WB_OK | WB_DEF_OK, IDE_RESSTR(RID_STR_SBXNAMEALLREADYUSED) ).Execute();
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
            ErrorBox( this, WB_OK | WB_DEF_OK, IDE_RESSTR(RID_STR_LIBNAMETOLONG) ).Execute();
        else
            ErrorBox( this, WB_OK | WB_DEF_OK, IDE_RESSTR(RID_STR_BADSBXNAME) ).Execute();
    }

    return bValid;
}

//----------------------------------------------------------------------------
// NewObjectDialog
//----------------------------------------------------------------------------

IMPL_LINK_NOARG(NewObjectDialog, OkButtonHandler)
{
    if (IsValidSbxName(aEdit.GetText()))
        EndDialog(1);
    else
    {
        ErrorBox(this, WB_OK | WB_DEF_OK,
                 IDE_RESSTR(RID_STR_BADSBXNAME)).Execute();
        aEdit.GrabFocus();
    }
    return 0;
}

NewObjectDialog::NewObjectDialog(Window * pParent, ObjectMode::Mode eMode,
                                 bool bCheckName)
    : ModalDialog( pParent, IDEResId( RID_DLG_NEWLIB ) ),
        aText( this, IDEResId( RID_FT_NEWLIB ) ),
        aEdit( this, IDEResId( RID_ED_LIBNAME ) ),
        aOKButton( this, IDEResId( RID_PB_OK ) ),
        aCancelButton( this, IDEResId( RID_PB_CANCEL ) )
{
    FreeResource();
    aEdit.GrabFocus();

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
        aOKButton.SetClickHdl(LINK(this, NewObjectDialog, OkButtonHandler));
}

//----------------------------------------------------------------------------

NewObjectDialog::~NewObjectDialog()
{
}


//----------------------------------------------------------------------------
// GotoLineDialog
//----------------------------------------------------------------------------

GotoLineDialog::GotoLineDialog(Window * pParent )
    : ModalDialog( pParent, IDEResId( RID_DLG_GOTOLINE ) ),
        aText( this, IDEResId( RID_FT_LINE ) ),
        aEdit( this, IDEResId( RID_ED_LINE ) ),
        aOKButton( this, IDEResId( RID_PB_OK ) ),
        aCancelButton( this, IDEResId( RID_PB_CANCEL ) )
{
    FreeResource();
    aEdit.GrabFocus();

    SetText( IDE_RESSTR(RID_STR_GETLINE) );
    aOKButton.SetClickHdl(LINK(this, GotoLineDialog, OkButtonHandler));

}

sal_Int32 GotoLineDialog::GetLineNumber()
{
    return OUString( aEdit.GetText() ).toInt32();
}

IMPL_LINK_NOARG(GotoLineDialog, OkButtonHandler)
{
    if ( GetLineNumber() )
        EndDialog(1);
    else
        aEdit.SetText( aEdit.GetText(), Selection(0, aEdit.GetText().getLength() ));
    return 0;
}


//----------------------------------------------------------------------------
// ExportDialog
//----------------------------------------------------------------------------

IMPL_LINK_NOARG(ExportDialog, OkButtonHandler)
{
    mbExportAsPackage = maExportAsPackageButton.IsChecked();
    EndDialog(1);
    return 0;
}

ExportDialog::ExportDialog( Window * pParent )
    : ModalDialog( pParent, IDEResId( RID_DLG_EXPORT ) )
    , maExportAsPackageButton( this, IDEResId( RB_EXPORTASPACKAGE ) )
    , maExportAsBasicButton( this, IDEResId( RB_EXPORTASBASIC ) )
    , maOKButton( this, IDEResId( RID_PB_OK ) )
    , maCancelButton( this, IDEResId( RID_PB_CANCEL ) )
    , mbExportAsPackage(false)
{
    FreeResource();
    maExportAsPackageButton.Check();
    maOKButton.SetClickHdl(LINK(this, ExportDialog, OkButtonHandler));
}

//----------------------------------------------------------------------------

ExportDialog::~ExportDialog()
{
}

//----------------------------------------------------------------------------
//  LibPage
//----------------------------------------------------------------------------

LibPage::LibPage( Window * pParent )
    :TabPage( pParent, IDEResId( RID_TP_LIBS ) )
    ,aBasicsText( this, IDEResId( RID_STR_BASICS ) )
    ,aBasicsBox( this, IDEResId( RID_LB_BASICS ) )
    ,aLibText( this, IDEResId( RID_STR_LIB ) )
    ,aLibBox( this, IDEResId( RID_TRLBOX ) )
    ,aEditButton( this, IDEResId( RID_PB_EDIT ) )
    ,aCloseButton( this, IDEResId( RID_PB_CLOSE ) )
    ,aPasswordButton( this, IDEResId( RID_PB_PASSWORD ) )
    ,aNewLibButton( this, IDEResId( RID_PB_NEWLIB ) )
    ,aInsertLibButton( this, IDEResId( RID_PB_APPEND ) )
    ,aExportButton( this, IDEResId( RID_PB_EXPORT ) )
    ,aDelButton( this, IDEResId( RID_PB_DELETE ) )
    ,m_aCurDocument( ScriptDocument::getApplicationScriptDocument() )
    ,m_eCurLocation( LIBRARY_LOCATION_UNKNOWN )
{
    FreeResource();
    pTabDlg = 0;

    aEditButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aNewLibButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aPasswordButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aExportButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aInsertLibButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aDelButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aCloseButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aLibBox.SetSelectHdl( LINK( this, LibPage, TreeListHighlightHdl ) );

    aBasicsBox.SetSelectHdl( LINK( this, LibPage, BasicSelectHdl ) );

    aLibBox.SetMode(ObjectMode::Module);
    aLibBox.EnableInplaceEditing(true);
    aLibBox.SetStyle( WB_HSCROLL | WB_BORDER | WB_TABSTOP );
    aCloseButton.GrabFocus();

    long aTabs[] = { 2, 30, 120 };
    aLibBox.SetTabs( aTabs, MAP_PIXEL );

    FillListBox();
    aBasicsBox.SelectEntryPos( 0 );
    SetCurLib();

    CheckButtons();
}

//----------------------------------------------------------------------------

LibPage::~LibPage()
{
    sal_uInt16 nCount = aBasicsBox.GetEntryCount();
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        DocumentEntry* pEntry = (DocumentEntry*)aBasicsBox.GetEntryData( i );
        delete pEntry;
    }
}

//----------------------------------------------------------------------------

void LibPage::CheckButtons()
{
    SvTreeListEntry* pCur = aLibBox.GetCurEntry();
    if ( pCur )
    {
        OUString aLibName = aLibBox.GetEntryText( pCur, 0 );
        Reference< script::XLibraryContainer2 > xModLibContainer( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
        Reference< script::XLibraryContainer2 > xDlgLibContainer( m_aCurDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );

        if ( m_eCurLocation == LIBRARY_LOCATION_SHARE )
        {
            aPasswordButton.Disable();
            aNewLibButton.Disable();
            aInsertLibButton.Disable();
            aDelButton.Disable();
        }
        else if ( aLibName.equalsIgnoreAsciiCase( "Standard" ) )
        {
            aPasswordButton.Disable();
            aNewLibButton.Enable();
            aInsertLibButton.Enable();
            aExportButton.Disable();
            aDelButton.Disable();
            if ( !aLibBox.HasFocus() )
                aCloseButton.GrabFocus();
        }
        else if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) ) ||
                  ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) ) )
        {
            aPasswordButton.Disable();
            aNewLibButton.Enable();
            aInsertLibButton.Enable();
            if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) && !xModLibContainer->isLibraryLink( aLibName ) ) ||
                 ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) && !xDlgLibContainer->isLibraryLink( aLibName ) ) )
                aDelButton.Disable();
            else
                aDelButton.Enable();
        }
        else
        {
            if ( xModLibContainer.is() && !xModLibContainer->hasByName( aLibName ) )
                aPasswordButton.Disable();
            else
                aPasswordButton.Enable();

            aNewLibButton.Enable();
            aInsertLibButton.Enable();
            aExportButton.Enable();
            aDelButton.Enable();
        }
    }
}

//----------------------------------------------------------------------------

void LibPage::ActivatePage()
{
    SetCurLib();
}

//----------------------------------------------------------------------------


void LibPage::DeactivatePage()
{
}

//----------------------------------------------------------------------------


IMPL_LINK_INLINE_START( LibPage, TreeListHighlightHdl, SvTreeListBox *, pBox )
{
    if ( pBox->IsSelected( pBox->GetHdlEntry() ) )
        CheckButtons();
    return 0;
}
IMPL_LINK_INLINE_END( LibPage, TreeListHighlightHdl, SvTreeListBox *, pBox )

//----------------------------------------------------------------------------

IMPL_LINK_INLINE_START( LibPage, BasicSelectHdl, ListBox *, pBox )
{
    (void)pBox;
    SetCurLib();
    CheckButtons();
    return 0;
}
IMPL_LINK_INLINE_END( LibPage, BasicSelectHdl, ListBox *, pBox )

//----------------------------------------------------------------------------

IMPL_LINK( LibPage, ButtonHdl, Button *, pButton )
{
    if ( pButton == &aEditButton )
    {
        SfxAllItemSet aArgs( SFX_APP()->GetPool() );
        SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
        SFX_APP()->ExecuteSlot( aRequest );

        SfxUsrAnyItem aDocItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, makeAny( m_aCurDocument.getDocumentOrNull() ) );
        SvTreeListEntry* pCurEntry = aLibBox.GetCurEntry();
        DBG_ASSERT( pCurEntry, "Entry?!" );
        OUString aLibName( aLibBox.GetEntryText( pCurEntry, 0 ) );
        SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
        if (SfxDispatcher* pDispatcher = GetDispatcher())
            pDispatcher->Execute( SID_BASICIDE_LIBSELECTED,
                                    SFX_CALLMODE_ASYNCHRON, &aDocItem, &aLibNameItem, 0L );
        EndTabDialog( 1 );
        return 0;
    }
    else if ( pButton == &aNewLibButton )
        NewLib();
    else if ( pButton == &aInsertLibButton )
        InsertLib();
    else if ( pButton == &aExportButton )
        Export();
    else if ( pButton == &aDelButton )
        DeleteCurrent();
    else if ( pButton == &aCloseButton )
    {
        EndTabDialog( 0 );
        return 0;
    }
    else if ( pButton == &aPasswordButton )
    {
        SvTreeListEntry* pCurEntry = aLibBox.GetCurEntry();
        OUString aLibName( aLibBox.GetEntryText( pCurEntry, 0 ) );

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
                SvxPasswordDialog* pDlg = new SvxPasswordDialog( this, true, !bProtected );
                pDlg->SetCheckPasswordHdl( LINK( this, LibPage, CheckPasswordHdl ) );

                if ( pDlg->Execute() == RET_OK )
                {
                    bool const bNewProtected = xPasswd->isLibraryPasswordProtected( aLibName );

                    if ( bNewProtected != bProtected )
                    {
                        sal_uLong nPos = (sal_uLong)aLibBox.GetModel()->GetAbsPos( pCurEntry );
                        aLibBox.GetModel()->Remove( pCurEntry );
                        ImpInsertLibEntry( aLibName, nPos );
                        aLibBox.SetCurEntry( aLibBox.GetEntry( nPos ) );
                    }

                    MarkDocumentModified( m_aCurDocument );
                }
                delete pDlg;
            }
        }
    }
    CheckButtons();
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK_INLINE_START( LibPage, CheckPasswordHdl, SvxPasswordDialog *, pDlg )
{
    long nRet = 0;

    SvTreeListEntry* pCurEntry = aLibBox.GetCurEntry();
    OUString aLibName( aLibBox.GetEntryText( pCurEntry, 0 ) );
    Reference< script::XLibraryContainerPassword > xPasswd( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );

    if ( xPasswd.is() )
    {
        try
        {
            OUString aOldPassword( pDlg->GetOldPassword() );
            OUString aNewPassword( pDlg->GetNewPassword() );
            xPasswd->changeLibraryPassword( aLibName, aOldPassword, aNewPassword );
            nRet = 1;
        }
        catch (...)
        {
        }
    }

    return nRet;
}
IMPL_LINK_INLINE_END( LibPage, CheckPasswordHdl, SvxPasswordDialog *, pDlg )

//----------------------------------------------------------------------------

void LibPage::NewLib()
{
    createLibImpl( static_cast<Window*>( this ), m_aCurDocument, &aLibBox, NULL);
}

//----------------------------------------------------------------------------

void LibPage::InsertLib()
{
    Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    // file open dialog
    Reference < XFilePicker3 > xFP = FilePicker::createWithMode(xContext, TemplateDescription::FILEOPEN_SIMPLE);
    xFP->setTitle(IDEResId(RID_STR_APPENDLIBS).toString());

    // filter
    OUString aTitle(IDEResId(RID_STR_BASIC).toString());
    OUString aFilter;
    aFilter =  "*.sbl;*.xlc;*.xlb" ;        // library files
    aFilter += ";*.sdw;*.sxw;*.odt" ;       // text
    aFilter += ";*.vor;*.stw;*.ott" ;       // text template
    aFilter += ";*.sgl;*.sxg;*.odm" ;       // master document
    aFilter += ";*.oth" ;                   // html document template
    aFilter += ";*.sdc;*.sxc;*.ods" ;       // spreadsheet
    aFilter += ";*.stc;*.ots" ;             // spreadsheet template
    aFilter += ";*.sda;*.sxd;*.odg" ;       // drawing
    aFilter += ";*.std;*.otg" ;             // drawing template
    aFilter += ";*.sdd;*.sxi;*.odp" ;       // presentation
    aFilter += ";*.sti;*.otp" ;             // presentation template
    aFilter += ";*.sxm;*.odf" ;             // formula
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
        Sequence< OUString > aFiles = xFP->getFiles();
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

        OUString aModURL( aModURLObj.GetMainURL( INetURLObject::NO_DECODE ) );
        if ( xSFA->exists( aModURL ) )
        {
            xModLibContImport = Reference< script::XLibraryContainer2 >(
                        script::DocumentScriptLibraryContainer::createWithURL(xContext, aModURL), UNO_QUERY );
        }

        OUString aDlgURL( aDlgURLObj.GetMainURL( INetURLObject::NO_DECODE ) );
        if ( xSFA->exists( aDlgURL ) )
        {
            xDlgLibContImport = Reference< script::XLibraryContainer2 >(
                        script::DocumentDialogLibraryContainer::createWithURL(xContext, aDlgURL), UNO_QUERY );
        }

        if ( xModLibContImport.is() || xDlgLibContImport.is() )
        {
            LibDialog* pLibDlg = 0;

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
                    pLibDlg = new LibDialog( this );
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
                InfoBox( this, IDE_RESSTR(RID_STR_NOLIBINSTORAGE) ).Execute();
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
                    sal_uLong nNewPos = aLibBox.GetEntryCount();
                    bool bRemove = false;
                    bool bReplace = pLibDlg->IsReplace();
                    bool bReference = pLibDlg->IsReference();
                    for ( sal_uInt16 nLib = 0; nLib < pLibDlg->GetLibBox().GetEntryCount(); nLib++ )
                    {
                        if ( pLibDlg->GetLibBox().IsChecked( nLib ) )
                        {
                            SvTreeListEntry* pEntry = pLibDlg->GetLibBox().GetEntry( nLib );
                            DBG_ASSERT( pEntry, "Entry?!" );
                            OUString aLibName( pLibDlg->GetLibBox().GetEntryText( pEntry, 0 ) );
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
                                        ErrorBox( this, WB_OK | WB_DEF_OK, IDE_RESSTR(RID_STR_REPLACESTDLIB) ).Execute();
                                        continue;
                                    }

                                    // check, if the library is readonly and not a link
                                    if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) && !xModLibContainer->isLibraryLink( aLibName ) ) ||
                                         ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) && !xDlgLibContainer->isLibraryLink( aLibName ) ) )
                                    {
                                        OUString aErrStr( IDE_RESSTR(RID_STR_REPLACELIB) );
                                        aErrStr = aErrStr.replaceAll("XX", aLibName);
                                        aErrStr += "\n";
                                        aErrStr += IDE_RESSTR(RID_STR_LIBISREADONLY);
                                        ErrorBox( this, WB_OK | WB_DEF_OK, aErrStr ).Execute();
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
                                    aErrStr = aErrStr.replaceAll("XX", aLibName);
                                    aErrStr += "\n" ;
                                    aErrStr += IDE_RESSTR(RID_STR_SBXNAMEALLREADYUSED);
                                    ErrorBox( this, WB_OK | WB_DEF_OK, aErrStr ).Execute();
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
                                        ErrorBox( this, WB_OK | WB_DEF_OK, aErrStr ).Execute();
                                        continue;
                                    }
                                }
                            }

                            // remove existing libraries
                            if ( bRemove )
                            {
                                // remove listbox entry
                                SvTreeListEntry* pEntry_ = aLibBox.FindEntry( aLibName );
                                if ( pEntry_ )
                                    aLibBox.SvTreeListBox::GetModel()->Remove( pEntry_ );

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
                                    OUString aModStorageURL( aModStorageURLObj.GetMainURL( INetURLObject::NO_DECODE ) );

                                    // create library link
                                    xModLib = Reference< container::XNameContainer >( xModLibContainer->createLibraryLink( aLibName, aModStorageURL, true ), UNO_QUERY);
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
                                                        OUString _aPassword( aPassword );
                                                        xPasswd->changeLibraryPassword( aLibName, OUString(), _aPassword );
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
                                    OUString aDlgStorageURL( aDlgStorageURLObj.GetMainURL( INetURLObject::NO_DECODE ) );

                                    // create library link
                                    xDlgLib = Reference< container::XNameContainer >( xDlgLibContainer->createLibraryLink( aLibName, aDlgStorageURL, true ), UNO_QUERY);
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
                            ImpInsertLibEntry( aLibName, aLibBox.GetEntryCount() );
                            bChanges = true;
                        }
                    }

                    SvTreeListEntry* pFirstNew = aLibBox.GetEntry( nNewPos );
                    if ( pFirstNew )
                        aLibBox.SetCurEntry( pFirstNew );
                }

                delete pLibDlg;
                if ( bChanges )
                    MarkDocumentModified( m_aCurDocument );
            }
        }
    }
}

//----------------------------------------------------------------------------

void LibPage::Export( void )
{
    SvTreeListEntry* pCurEntry = aLibBox.GetCurEntry();
    OUString aLibName( aLibBox.GetEntryText( pCurEntry, 0 ) );

    // Password verification
    OUString aOULibName( aLibName );
    Reference< script::XLibraryContainer2 > xModLibContainer( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );

    if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
    {
        bool bOK = true;

        // check password
        Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
        if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
        {
            OUString aPassword;
            Reference< script::XLibraryContainer > xModLibContainer1( xModLibContainer, UNO_QUERY );
            bOK = QueryPassword( xModLibContainer1, aLibName, aPassword );
        }
        if ( !bOK )
            return;
    }


    ExportDialog aNewDlg(this);
    if (aNewDlg.Execute() == RET_OK)
    {
        try
        {
            if (aNewDlg.isExportAsPackage())
                ExportAsPackage( aLibName );
            else
                ExportAsBasic( aLibName );
        }
        catch(const util::VetoException& ) // user cancled operation
        {
        }
    }
}

void LibPage::implExportLib( const OUString& aLibName, const OUString& aTargetURL,
    const Reference< task::XInteractionHandler >& Handler )
{
    OUString aOULibName( aLibName );
    Reference< script::XLibraryContainerExport > xModLibContainerExport
        ( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
    Reference< script::XLibraryContainerExport > xDlgLibContainerExport
        ( m_aCurDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
    if ( xModLibContainerExport.is() )
        xModLibContainerExport->exportLibrary( aOULibName, aTargetURL, Handler );

    if ( xDlgLibContainerExport.is() )
        xDlgLibContainerExport->exportLibrary( aOULibName, aTargetURL, Handler );
}


//===========================================================================
// Implementation XCommandEnvironment

typedef cppu::WeakImplHelper1< XCommandEnvironment > LibCommandEnvironmentHelper;

class OLibCommandEnvironment : public LibCommandEnvironmentHelper
{
    Reference< task::XInteractionHandler > mxInteraction;

public:
    OLibCommandEnvironment( Reference< task::XInteractionHandler > xInteraction )
        : mxInteraction( xInteraction )
    {}

    // Methods
    virtual Reference< task::XInteractionHandler > SAL_CALL getInteractionHandler()
        throw(RuntimeException);
    virtual Reference< XProgressHandler > SAL_CALL getProgressHandler()
        throw(RuntimeException);
};

Reference< task::XInteractionHandler > OLibCommandEnvironment::getInteractionHandler()
    throw(RuntimeException)
{
    return mxInteraction;
}

Reference< XProgressHandler > OLibCommandEnvironment::getProgressHandler()
    throw(RuntimeException)
{
    Reference< XProgressHandler > xRet;
    return xRet;
}



void LibPage::ExportAsPackage( const OUString& aLibName )
{
    // file open dialog
    Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference< task::XInteractionHandler2 > xHandler( task::InteractionHandler::createWithParent(xContext, 0) );
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

        Sequence< OUString > aFiles = xFP->getFiles();
        INetURLObject aURL( aFiles[0] );
        if( aURL.getExtension().isEmpty() )
            aURL.setExtension( "oxt" );

        OUString aPackageURL( aURL.GetMainURL( INetURLObject::NO_DECODE ) );

        OUString aTmpPath = SvtPathOptions().GetTempPath();
        INetURLObject aInetObj( aTmpPath );
        aInetObj.insertName( aLibName, true, INetURLObject::LAST_SEGMENT, true, INetURLObject::ENCODE_ALL );
        OUString aSourcePath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
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
            sourceContent, ::ucbhelper::InsertOperation_COPY,
            OUString(), NameClash::OVERWRITE );

        INetURLObject aMetaInfInetObj( aTmpPath );
        aMetaInfInetObj.insertName( "META-INF",
            true, INetURLObject::LAST_SEGMENT, true, INetURLObject::ENCODE_ALL );
        OUString aMetaInfFolder = aMetaInfInetObj.GetMainURL( INetURLObject::NO_DECODE );
        if( xSFA->exists( aMetaInfFolder ) )
            xSFA->kill( aMetaInfFolder );
        xSFA->createFolder( aMetaInfFolder );

        ::std::vector< Sequence<beans::PropertyValue> > manifest;
        const OUString strMediaType = "MediaType" ;
        const OUString strFullPath = "FullPath" ;
        const OUString strBasicMediaType = "application/vnd.sun.star.basic-library" ;

        Sequence<beans::PropertyValue> attribs( 2 );
        beans::PropertyValue * pattribs = attribs.getArray();
        pattribs[ 0 ].Name = strFullPath;
        OUString fullPath = aLibName;
        fullPath += "/" ;
        pattribs[ 0 ].Value <<= fullPath;
        pattribs[ 1 ].Name = strMediaType;
        pattribs[ 1 ].Value <<= strBasicMediaType;
        manifest.push_back( attribs );

        // write into pipe:
        Reference<packages::manifest::XManifestWriter> xManifestWriter = packages::manifest::ManifestWriter::create( xContext );
        Reference<io::XOutputStream> xPipe( io::Pipe::create( xContext ), UNO_QUERY_THROW );
        xManifestWriter->writeManifestSequence(
            xPipe, Sequence< Sequence<beans::PropertyValue> >(
                &manifest[ 0 ], manifest.size() ) );

        aMetaInfInetObj.insertName( "manifest.xml",
            true, INetURLObject::LAST_SEGMENT, true, INetURLObject::ENCODE_ALL );

        // write buffered pipe data to content:
        ::ucbhelper::Content manifestContent( aMetaInfInetObj.GetMainURL( INetURLObject::NO_DECODE ), xCmdEnv, comphelper::getProcessComponentContext() );
        manifestContent.writeStream( Reference<io::XInputStream>( xPipe, UNO_QUERY_THROW ), true );

        ::ucbhelper::Content MetaInfContent( aMetaInfFolder, xCmdEnv, comphelper::getProcessComponentContext() );
        destFolderContent.transferContent(
            MetaInfContent, ::ucbhelper::InsertOperation_COPY,
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
    Reference< task::XInteractionHandler2 > xHandler( task::InteractionHandler::createWithParent(xContext, 0) );

    xFolderPicker->setTitle(IDEResId(RID_STR_EXPORTBASIC).toString());

    // set display directory and filter
    OUString aPath =GetExtraData()->GetAddLibPath();
    if( aPath.isEmpty() )
        aPath = SvtPathOptions().GetWorkPath();

    // INetURLObject aURL(m_sSavePath, INET_PROT_FILE);
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

//----------------------------------------------------------------------------

void LibPage::DeleteCurrent()
{
    SvTreeListEntry* pCurEntry = aLibBox.GetCurEntry();
    OUString aLibName( aLibBox.GetEntryText( pCurEntry, 0 ) );

    // check, if library is link
    bool bIsLibraryLink = false;
    OUString aOULibName( aLibName );
    Reference< script::XLibraryContainer2 > xModLibContainer( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
    Reference< script::XLibraryContainer2 > xDlgLibContainer( m_aCurDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
    if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryLink( aOULibName ) ) ||
         ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryLink( aOULibName ) ) )
    {
        bIsLibraryLink = true;
    }

    if ( QueryDelLib( aLibName, bIsLibraryLink, this ) )
    {
        // inform BasicIDE
        SfxUsrAnyItem aDocItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, makeAny( m_aCurDocument.getDocumentOrNull() ) );
        SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
        if (SfxDispatcher* pDispatcher = GetDispatcher())
            pDispatcher->Execute( SID_BASICIDE_LIBREMOVED,
                                  SFX_CALLMODE_SYNCHRON, &aDocItem, &aLibNameItem, 0L );

        // remove library from module and dialog library containers
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
            xModLibContainer->removeLibrary( aOULibName );
        if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) )
            xDlgLibContainer->removeLibrary( aOULibName );

        static_cast<SvTreeListBox&>(aLibBox).GetModel()->Remove( pCurEntry );
        MarkDocumentModified( m_aCurDocument );
    }
}

//----------------------------------------------------------------------------

void LibPage::EndTabDialog( sal_uInt16 nRet )
{
    DBG_ASSERT( pTabDlg, "TabDlg nicht gesetzt!" );
    if ( pTabDlg )
        pTabDlg->EndDialog( nRet );
}

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

void LibPage::InsertListBoxEntry( const ScriptDocument& rDocument, LibraryLocation eLocation )
{
    OUString aEntryText( rDocument.getTitle( eLocation ) );
    sal_uInt16 nPos = aBasicsBox.InsertEntry( aEntryText, LISTBOX_APPEND );
    aBasicsBox.SetEntryData( nPos, new DocumentEntry(rDocument, eLocation) );
}

//----------------------------------------------------------------------------

void LibPage::SetCurLib()
{
    sal_uInt16 nSelPos = aBasicsBox.GetSelectEntryPos();
    DocumentEntry* pEntry = (DocumentEntry*)aBasicsBox.GetEntryData( nSelPos );
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
            aLibBox.SetDocument( aDocument );
            aLibBox.Clear();

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

            SvTreeListEntry* pEntry_ = aLibBox.FindEntry( OUString( "Standard" ) );
            if ( !pEntry_ )
                pEntry_ = aLibBox.GetEntry( 0 );
            aLibBox.SetCurEntry( pEntry_ );
        }
    }
}

//----------------------------------------------------------------------------

SvTreeListEntry* LibPage::ImpInsertLibEntry( const OUString& rLibName, sal_uLong nPos )
{
    // check, if library is password protected
    bool bProtected = false;
    OUString aOULibName( rLibName );
    Reference< script::XLibraryContainer2 > xModLibContainer( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
    {
        Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
        if ( xPasswd.is() )
        {
            bProtected = xPasswd->isLibraryPasswordProtected( aOULibName );
        }
    }

    SvTreeListEntry* pNewEntry = aLibBox.DoInsertEntry( rLibName, nPos );
    pNewEntry->SetUserData( new LibUserData(m_aCurDocument) );

    if (bProtected)
    {
        Image aImage(IDEResId(RID_IMG_LOCKED));
        aLibBox.SetExpandedEntryBmp(pNewEntry, aImage);
        aLibBox.SetCollapsedEntryBmp(pNewEntry, aImage);
    }

    // check, if library is link
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryLink( aOULibName ) )
    {
        OUString aLinkURL = xModLibContainer->getLibraryLinkURL( aOULibName );
        aLibBox.SetEntryText( aLinkURL, pNewEntry, 1 );
    }

    return pNewEntry;
}

//----------------------------------------------------------------------------

// Helper function
void createLibImpl( Window* pWin, const ScriptDocument& rDocument,
                    CheckBox* pLibBox, TreeListBox* pBasicBox )
{
    OSL_ENSURE( rDocument.isAlive(), "createLibImpl: invalid document!" );
    if ( !rDocument.isAlive() )
        return;

    // create library name
    OUString aLibName;
    OUString aLibStdName( "Library" );
    //String aLibStdName( IDEResId( RID_STR_STDLIBNAME ) );
    bool bValid = false;
    sal_Int32 i = 1;
    while ( !bValid )
    {
        aLibName = aLibStdName + OUString::number( i );
        if ( !rDocument.hasLibrary( E_SCRIPTS, aLibName ) && !rDocument.hasLibrary( E_DIALOGS, aLibName ) )
            bValid = true;
        i++;
    }

    NewObjectDialog aNewDlg(pWin, ObjectMode::Library);
    aNewDlg.SetObjectName(aLibName);

    if (aNewDlg.Execute())
    {
        if (!aNewDlg.GetObjectName().isEmpty())
            aLibName = aNewDlg.GetObjectName();

        if ( aLibName.getLength() > 30 )
        {
            ErrorBox( pWin, WB_OK | WB_DEF_OK, IDEResId(RID_STR_LIBNAMETOLONG).toString() ).Execute();
        }
        else if ( !IsValidSbxName( aLibName ) )
        {
            ErrorBox( pWin, WB_OK | WB_DEF_OK,
                        IDEResId(RID_STR_BADSBXNAME).toString() ).Execute();
        }
        else if ( rDocument.hasLibrary( E_SCRIPTS, aLibName ) || rDocument.hasLibrary( E_DIALOGS, aLibName ) )
        {
            ErrorBox( pWin, WB_OK | WB_DEF_OK,
                        IDEResId(RID_STR_SBXNAMEALLREADYUSED2).toString() ).Execute();
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
                    pDispatcher->Execute( SID_BASICIDE_SBXINSERTED,
                                          SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );

                if( pBasicBox )
                {
                    SvTreeListEntry* pEntry = pBasicBox->GetCurEntry();
                    SvTreeListEntry* pRootEntry = NULL;
                    while( pEntry )
                    {
                        pRootEntry = pEntry;
                        pEntry = pBasicBox->GetParent( pEntry );
                    }

                    sal_uInt16 nMode = pBasicBox->GetMode();
                    bool bDlgMode = ( nMode & BROWSEMODE_DIALOGS ) && !( nMode & BROWSEMODE_MODULES );
                    sal_uInt16 nId = bDlgMode ? RID_IMG_DLGLIB : RID_IMG_MODLIB;
                    SAL_WNODEPRECATED_DECLARATIONS_PUSH
                    SvTreeListEntry* pNewLibEntry = pBasicBox->AddEntry(
                        aLibName,
                        Image( IDEResId( nId ) ),
                        pRootEntry, false,
                        std::auto_ptr<Entry>(new Entry(OBJ_TYPE_LIBRARY))
                    );
                    SAL_WNODEPRECATED_DECLARATIONS_POP
                    DBG_ASSERT( pNewLibEntry, "InsertEntry fehlgeschlagen!" );

                    if( pNewLibEntry )
                    {
                        SAL_WNODEPRECATED_DECLARATIONS_PUSH
                        SvTreeListEntry* pEntry_ = pBasicBox->AddEntry(
                            aModName,
                            Image( IDEResId( RID_IMG_MODULE ) ),
                            pNewLibEntry, false,
                            std::auto_ptr<Entry>(new Entry(OBJ_TYPE_MODULE))
                        );
                        SAL_WNODEPRECATED_DECLARATIONS_POP
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

//----------------------------------------------------------------------------

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
