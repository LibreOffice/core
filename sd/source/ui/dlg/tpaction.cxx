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

#include <svx/svxids.hrc>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/embed/NeedsRunningStateException.hpp>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <comphelper/string.hxx>
#include <com/sun/star/embed/VerbAttributes.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <sdattr.hrc>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>

#include <vcl/waitobj.hxx>
#include <sfx2/app.hxx>
#include <unotools/pathoptions.hxx>
#include <svx/svdograf.hxx>
#include <svl/aeitem.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/docfile.hxx>
#include <svx/xtable.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/lstbox.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svx/drawitem.hxx>
#include <View.hxx>
#include <sdresid.hxx>
#include <tpaction.hxx>
#include <strmname.h>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <strings.hrc>

#include <filedlg.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

#define DOCUMENT_TOKEN '#'

/**
 * Constructor of the Tab dialog: appends the pages to the dialog
 */
SdActionDlg::SdActionDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View const * pView)
    : SfxSingleTabDialogController(pParent, *pAttr, "modules/simpress/ui/interactiondialog.ui",
                                   "InteractionDialog")
    , rOutAttrs(*pAttr)
{
    TabPageParent aParent(get_content_area(), this);
    VclPtr<SfxTabPage> xNewPage = SdTPAction::Create(aParent, rOutAttrs);

    // formerly in PageCreated
    static_cast<SdTPAction*>( xNewPage.get() )->SetView( pView );
    static_cast<SdTPAction*>( xNewPage.get() )->Construct();

    SetTabPage(xNewPage);
}

/**
 *  Action-TabPage
 */
SdTPAction::SdTPAction(TabPageParent pWindow, const SfxItemSet& rInAttrs)
    : SfxTabPage(pWindow, "modules/simpress/ui/interactionpage.ui", "InteractionPage", &rInAttrs)
    , mpView(nullptr)
    , mpDoc(nullptr)
    , bTreeUpdated(false)
    , m_xLbAction(m_xBuilder->weld_combo_box("listbox"))
    , m_xFtTree(m_xBuilder->weld_label("fttree"))
    , m_xLbTree(new SdPageObjsTLV(m_xBuilder->weld_tree_view("tree")))
    , m_xLbTreeDocument(new SdPageObjsTLV(m_xBuilder->weld_tree_view("treedoc")))
    , m_xLbOLEAction(m_xBuilder->weld_tree_view("oleaction"))
    , m_xFrame(m_xBuilder->weld_frame("frame"))
    , m_xEdtSound(m_xBuilder->weld_entry("sound"))
    , m_xEdtBookmark(m_xBuilder->weld_entry("bookmark"))
    , m_xEdtDocument(m_xBuilder->weld_entry("document"))
    , m_xEdtProgram(m_xBuilder->weld_entry("program"))
    , m_xEdtMacro(m_xBuilder->weld_entry("macro"))
    , m_xBtnSearch(m_xBuilder->weld_button("browse"))
    , m_xBtnSeek(m_xBuilder->weld_button("find"))
{
    m_xLbOLEAction->set_size_request(m_xLbOLEAction->get_approximate_digit_width() * 48,
                                     m_xLbOLEAction->get_height_rows(12));

    m_xBtnSearch->connect_clicked( LINK( this, SdTPAction, ClickSearchHdl ) );
    m_xBtnSeek->connect_clicked( LINK( this, SdTPAction, ClickSearchHdl ) );

    // this page needs ExchangeSupport
    SetExchangeSupport();

    m_xLbAction->connect_changed( LINK( this, SdTPAction, ClickActionHdl ) );
    m_xLbTree->connect_changed( LINK( this, SdTPAction, SelectTreeHdl ) );
    m_xEdtDocument->connect_focus_out( LINK( this, SdTPAction, CheckFileHdl ) );
    m_xEdtMacro->connect_focus_out( LINK( this, SdTPAction, CheckFileHdl ) );

    //Lock to initial max size
    Size aSize(m_xContainer->get_preferred_size());
    m_xContainer->set_size_request(aSize.Width(), aSize.Height());

    ClickActionHdl( *m_xLbAction );
}

SdTPAction::~SdTPAction()
{
    disposeOnce();
}

void SdTPAction::SetView( const ::sd::View* pSdView )
{
    mpView = pSdView;

    // get ColorTable and fill ListBox
    ::sd::DrawDocShell* pDocSh = mpView->GetDocSh();
    if( pDocSh && pDocSh->GetViewShell() )
    {
        mpDoc = pDocSh->GetDoc();
        SfxViewFrame* pFrame = pDocSh->GetViewShell()->GetViewFrame();
        m_xLbTree->SetViewFrame( pFrame );
        m_xLbTreeDocument->SetViewFrame( pFrame );

        pColList = pDocSh->GetItem( SID_COLOR_TABLE )->GetColorList();
        DBG_ASSERT( pColList.is(), "No color table available!" );
    }
    else
    {
        OSL_FAIL("sd::SdTPAction::SetView(), no docshell or viewshell?");
    }
}

void SdTPAction::Construct()
{
    // fill OLE-Actionlistbox
    SdrOle2Obj* pOleObj = nullptr;
    SdrGrafObj* pGrafObj = nullptr;
    bool        bOLEAction = false;

    if ( mpView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
        SdrObject* pObj;

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            pObj = pMark->GetMarkedSdrObj();

            SdrInventor nInv        = pObj->GetObjInventor();
            sal_uInt16  nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv == SdrInventor::Default && nSdrObjKind == OBJ_OLE2)
            {
                pOleObj = static_cast<SdrOle2Obj*>(pObj);
            }
            else if (nInv == SdrInventor::Default && nSdrObjKind == OBJ_GRAF)
            {
                pGrafObj = static_cast<SdrGrafObj*>(pObj);
            }
        }
    }
    if( pGrafObj )
    {
        bOLEAction = true;

        aVerbVector.push_back( 0 );
        m_xLbOLEAction->append_text( MnemonicGenerator::EraseAllMnemonicChars( SdResId( STR_EDIT_OBJ ) ) );
    }
    else if( pOleObj )
    {
        const uno::Reference < embed::XEmbeddedObject >& xObj = pOleObj->GetObjRef();
        if ( xObj.is() )
        {
            bOLEAction = true;
            uno::Sequence < embed::VerbDescriptor > aVerbs;
            try
            {
                aVerbs = xObj->getSupportedVerbs();
            }
            catch ( embed::NeedsRunningStateException& )
            {
                xObj->changeState( embed::EmbedStates::RUNNING );
                aVerbs = xObj->getSupportedVerbs();
            }

            for( sal_Int32 i=0; i<aVerbs.getLength(); i++ )
            {
                embed::VerbDescriptor aVerb = aVerbs[i];
                if( aVerb.VerbAttributes & embed::VerbAttributes::MS_VERBATTR_ONCONTAINERMENU )
                {
                    OUString aTmp( aVerb.VerbName );
                    aVerbVector.push_back( aVerb.VerbID );
                    m_xLbOLEAction->append_text( MnemonicGenerator::EraseAllMnemonicChars( aTmp ) );
                }
            }
        }
    }

    maCurrentActions.push_back( presentation::ClickAction_NONE );
    maCurrentActions.push_back( presentation::ClickAction_PREVPAGE );
    maCurrentActions.push_back( presentation::ClickAction_NEXTPAGE );
    maCurrentActions.push_back( presentation::ClickAction_FIRSTPAGE );
    maCurrentActions.push_back( presentation::ClickAction_LASTPAGE );
    maCurrentActions.push_back( presentation::ClickAction_BOOKMARK );
    maCurrentActions.push_back( presentation::ClickAction_DOCUMENT );
    maCurrentActions.push_back( presentation::ClickAction_SOUND );
    if( bOLEAction && m_xLbOLEAction->n_children() )
        maCurrentActions.push_back( presentation::ClickAction_VERB );
    maCurrentActions.push_back( presentation::ClickAction_PROGRAM );
    maCurrentActions.push_back( presentation::ClickAction_MACRO );
    maCurrentActions.push_back( presentation::ClickAction_STOPPRESENTATION );

    // fill Action-Listbox
    for (presentation::ClickAction & rAction : maCurrentActions)
    {
        const char* pRId = GetClickActionSdResId(rAction);
        m_xLbAction->append_text(SdResId(pRId));
    }

}

bool SdTPAction::FillItemSet( SfxItemSet* rAttrs )
{
    bool bModified = false;
    presentation::ClickAction eCA = presentation::ClickAction_NONE;

    if (m_xLbAction->get_active() != -1)
        eCA = GetActualClickAction();

    if( m_xLbAction->get_value_changed_from_saved() )
    {
        rAttrs->Put( SfxAllEnumItem( ATTR_ACTION, static_cast<sal_uInt16>(eCA) ) );
        bModified = true;
    }
    else
        rAttrs->InvalidateItem( ATTR_ACTION );

    OUString aFileName = GetEditText( true );
    if( aFileName.isEmpty() )
        rAttrs->InvalidateItem( ATTR_ACTION_FILENAME );
    else
    {
        if( mpDoc && mpDoc->GetDocSh() && mpDoc->GetDocSh()->GetMedium() )
        {
            OUString aBaseURL = mpDoc->GetDocSh()->GetMedium()->GetBaseURL();
            if( eCA == presentation::ClickAction_SOUND ||
                eCA == presentation::ClickAction_DOCUMENT ||
                eCA == presentation::ClickAction_PROGRAM )
                aFileName = ::URIHelper::SmartRel2Abs( INetURLObject(aBaseURL), aFileName, URIHelper::GetMaybeFileHdl(), true, false,
                                                        INetURLObject::EncodeMechanism::WasEncoded,
                                                        INetURLObject::DecodeMechanism::Unambiguous );

            rAttrs->Put( SfxStringItem( ATTR_ACTION_FILENAME, aFileName ) );
            bModified = true;
        }
        else
        {
            OSL_FAIL("sd::SdTPAction::FillItemSet(), I need a medium!");
        }
    }

    return bModified;
}

void SdTPAction::Reset( const SfxItemSet* rAttrs )
{
    presentation::ClickAction eCA = presentation::ClickAction_NONE;
    OUString aFileName;

    // m_xLbAction
    if( rAttrs->GetItemState( ATTR_ACTION ) != SfxItemState::DONTCARE )
    {
        eCA = static_cast<presentation::ClickAction>(static_cast<const SfxAllEnumItem&>( rAttrs->
                    Get( ATTR_ACTION ) ).GetValue());
        SetActualClickAction( eCA );
    }
    else
        m_xLbAction->set_active(-1);

    // m_xEdtSound
    if( rAttrs->GetItemState( ATTR_ACTION_FILENAME ) != SfxItemState::DONTCARE )
    {
            aFileName = static_cast<const SfxStringItem&>( rAttrs->Get( ATTR_ACTION_FILENAME ) ).GetValue();
            SetEditText( aFileName );
    }

    switch( eCA )
    {
        case presentation::ClickAction_BOOKMARK:
        {
            if (!m_xLbTree->SelectEntry(aFileName))
                m_xLbTree->unselect_all();
        }
        break;

        case presentation::ClickAction_DOCUMENT:
        {
            if( comphelper::string::getTokenCount(aFileName, DOCUMENT_TOKEN) == 2 )
                m_xLbTreeDocument->SelectEntry( aFileName.getToken( 1, DOCUMENT_TOKEN ) );
        }
        break;

        default:
        break;
    }
    ClickActionHdl( *m_xLbAction );

    m_xLbAction->save_value();
    m_xEdtSound->save_value();
}

void SdTPAction::ActivatePage( const SfxItemSet& )
{
}

DeactivateRC SdTPAction::DeactivatePage( SfxItemSet* pPageSet )
{
    if( pPageSet )
        FillItemSet( pPageSet );

    return DeactivateRC::LeavePage;
}

VclPtr<SfxTabPage> SdTPAction::Create( TabPageParent pParent,
                                       const SfxItemSet& rAttrs )
{
    return VclPtr<SdTPAction>::Create( pParent, rAttrs );
}

void SdTPAction::UpdateTree()
{
    if( !bTreeUpdated && mpDoc && mpDoc->GetDocSh() && mpDoc->GetDocSh()->GetMedium() )
    {
        m_xLbTree->Fill( mpDoc, true, mpDoc->GetDocSh()->GetMedium()->GetName() );
        bTreeUpdated = true;
    }
}

void SdTPAction::OpenFileDialog()
{
    // Soundpreview only for interaction with sound
    presentation::ClickAction eCA = GetActualClickAction();
    bool bSound = ( eCA == presentation::ClickAction_SOUND );
    bool bPage = ( eCA == presentation::ClickAction_BOOKMARK );
    bool bDocument = ( eCA == presentation::ClickAction_DOCUMENT ||
                       eCA == presentation::ClickAction_PROGRAM );
    bool bMacro = ( eCA == presentation::ClickAction_MACRO );

    if( bPage )
    {
        // search in the TreeLB for the specified object
        m_xLbTree->SelectEntry(GetEditText());
    }
    else
    {
        OUString aFile( GetEditText() );

        if (bSound)
        {
            SdOpenSoundFileDialog aFileDialog(GetDialogFrameWeld());

            if( !aFile.isEmpty() )
                aFileDialog.SetPath( aFile );

            if( aFileDialog.Execute() == ERRCODE_NONE )
            {
                aFile = aFileDialog.GetPath();
                SetEditText( aFile );
            }
        }
        else if (bMacro)
        {
            // choose macro dialog
            OUString aScriptURL = SfxApplication::ChooseScript(GetDialogFrameWeld());

            if ( !aScriptURL.isEmpty() )
            {
                SetEditText( aScriptURL );
            }
        }
        else
        {
            sfx2::FileDialogHelper aFileDialog(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION,
                FileDialogFlags::NONE, GetDialogFrameWeld());

            if (bDocument && aFile.isEmpty())
                aFile = SvtPathOptions().GetWorkPath();

            aFileDialog.SetDisplayDirectory( aFile );

            // The following is a workaround for #i4306#:
            // The addition of the implicitly existing "all files"
            // filter makes the (Windows system) open file dialog follow
            // links on the desktop to directories.
            aFileDialog.AddFilter (
                SfxResId(STR_SFX_FILTERNAME_ALL),
                "*.*");

            if( aFileDialog.Execute() == ERRCODE_NONE )
            {
                aFile = aFileDialog.GetPath();
                SetEditText( aFile );
            }
            if( bDocument )
                CheckFileHdl( *m_xEdtDocument );
        }
    }
}

IMPL_LINK_NOARG(SdTPAction, ClickSearchHdl, weld::Button&, void)
{
    OpenFileDialog();
}

IMPL_LINK_NOARG(SdTPAction, ClickActionHdl, weld::ComboBox&, void)
{
    presentation::ClickAction eCA = GetActualClickAction();

    // hide controls we don't need
    switch( eCA )
    {
        case presentation::ClickAction_NONE:
        case presentation::ClickAction_INVISIBLE:
        case presentation::ClickAction_PREVPAGE:
        case presentation::ClickAction_NEXTPAGE:
        case presentation::ClickAction_FIRSTPAGE:
        case presentation::ClickAction_LASTPAGE:
        case presentation::ClickAction_STOPPRESENTATION:
        default:
            m_xFtTree->hide();
            m_xLbTree->hide();
            m_xLbTreeDocument->hide();
            m_xLbOLEAction->hide();

            m_xFrame->hide();
            m_xEdtSound->hide();
            m_xEdtBookmark->hide();
            m_xEdtDocument->hide();
            m_xEdtProgram->hide();
            m_xEdtMacro->hide();
            m_xBtnSearch->hide();
            m_xBtnSeek->hide();
            break;

        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_PROGRAM:
        case presentation::ClickAction_MACRO:
            m_xFtTree->hide();
            m_xLbTree->hide();
            m_xLbTreeDocument->hide();
            m_xLbOLEAction->hide();

            m_xEdtDocument->hide();

            if( eCA == presentation::ClickAction_MACRO )
            {
                m_xEdtSound->hide();
                m_xEdtProgram->hide();
            }
            else if( eCA == presentation::ClickAction_PROGRAM )
            {
                m_xEdtSound->hide();
                m_xEdtMacro->hide();
            }
            else if( eCA == presentation::ClickAction_SOUND )
            {
                m_xEdtProgram->hide();
                m_xEdtMacro->hide();
            }

            m_xBtnSeek->hide();
            break;

        case presentation::ClickAction_DOCUMENT:
            m_xLbTree->hide();
            m_xLbOLEAction->hide();

            m_xEdtSound->hide();
            m_xEdtProgram->hide();
            m_xEdtMacro->hide();
            m_xEdtBookmark->hide();
            m_xBtnSeek->hide();
            break;

        case presentation::ClickAction_BOOKMARK:
            m_xLbTreeDocument->hide();
            m_xLbOLEAction->hide();
            m_xEdtSound->hide();
            m_xEdtDocument->hide();
            m_xEdtProgram->hide();
            m_xEdtMacro->hide();
            m_xBtnSearch->hide();
            break;

        case presentation::ClickAction_VERB:
            m_xLbTree->hide();
            m_xEdtDocument->hide();
            m_xEdtProgram->hide();
            m_xEdtBookmark->hide();
            m_xEdtMacro->hide();
            m_xBtnSearch->hide();
            m_xFrame->hide();
            m_xEdtSound->hide();
            m_xBtnSeek->hide();
            break;
    }

    // show controls we do need
    switch( eCA )
    {
        case presentation::ClickAction_NONE:
        case presentation::ClickAction_INVISIBLE:
        case presentation::ClickAction_PREVPAGE:
        case presentation::ClickAction_NEXTPAGE:
        case presentation::ClickAction_FIRSTPAGE:
        case presentation::ClickAction_LASTPAGE:
        case presentation::ClickAction_STOPPRESENTATION:
            // none
            break;

        case presentation::ClickAction_SOUND:
            m_xFrame->show();
            m_xEdtSound->show();
            m_xEdtSound->set_sensitive(true);
            m_xBtnSearch->show();
            m_xBtnSearch->set_sensitive(true);
            m_xFrame->set_label( SdResId( STR_EFFECTDLG_SOUND ) );
            break;

        case presentation::ClickAction_PROGRAM:
        case presentation::ClickAction_MACRO:
            m_xFrame->show();
            m_xBtnSearch->show();
            m_xBtnSearch->set_sensitive(true);
            if( eCA == presentation::ClickAction_MACRO )
            {
                m_xEdtMacro->show();
                m_xFrame->set_label( SdResId( STR_EFFECTDLG_MACRO ) );
            }
            else
            {
                m_xEdtProgram->show();
                m_xFrame->set_label( SdResId( STR_EFFECTDLG_PROGRAM ) );
            }
            break;

        case presentation::ClickAction_DOCUMENT:
            m_xFtTree->show();
            m_xLbTreeDocument->show();

            m_xFrame->show();
            m_xEdtDocument->show();
            m_xBtnSearch->show();
            m_xBtnSearch->set_sensitive(true);

            m_xFtTree->set_label( SdResId( STR_EFFECTDLG_JUMP ) );
            m_xFrame->set_label( SdResId( STR_EFFECTDLG_DOCUMENT ) );

            CheckFileHdl( *m_xEdtDocument );
            break;

        case presentation::ClickAction_VERB:
            m_xFtTree->show();
            m_xLbOLEAction->show();

            m_xFtTree->set_label( SdResId( STR_EFFECTDLG_ACTION ) );
            break;

        case presentation::ClickAction_BOOKMARK:
            UpdateTree();

            m_xFtTree->show();
            m_xLbTree->show();

            m_xFrame->show();
            m_xEdtBookmark->show();
            m_xBtnSeek->show();

            m_xFtTree->set_label( SdResId( STR_EFFECTDLG_JUMP ) );
            m_xFrame->set_label( SdResId( STR_EFFECTDLG_PAGE_OBJECT ) );
            break;
        default:
            break;
    }
}

IMPL_LINK_NOARG(SdTPAction, SelectTreeHdl, weld::TreeView&, void)
{
    m_xEdtBookmark->set_text( m_xLbTree->get_selected_text() );
}

IMPL_LINK_NOARG(SdTPAction, CheckFileHdl, weld::Widget&, void)
{
    OUString aFile( GetEditText() );

    if( aFile == aLastFile )
        return;

    // check if it is a valid draw file
    SfxMedium aMedium( aFile,
                StreamMode::READ | StreamMode::NOCREATE );

    if( aMedium.IsStorage() )
    {
        WaitObject aWait( GetParentDialog() );

        bool bHideTreeDocument = true;

        // is it a draw file?
        // open with READ, otherwise the Storages might write into the file!
        uno::Reference < embed::XStorage > xStorage = aMedium.GetStorage();
        DBG_ASSERT( xStorage.is(), "No storage!" );

        uno::Reference < container::XNameAccess > xAccess( xStorage, uno::UNO_QUERY );
        if (xAccess.is())
        {
            try
            {
                if (xAccess->hasByName(pStarDrawXMLContent) ||
                    xAccess->hasByName(pStarDrawOldXMLContent))
                {
                    if (SdDrawDocument* pBookmarkDoc = mpDoc->OpenBookmarkDoc(aFile))
                    {
                        aLastFile = aFile;

                        m_xLbTreeDocument->clear();
                        m_xLbTreeDocument->Fill(pBookmarkDoc, true, aFile);
                        mpDoc->CloseBookmarkDoc();
                        m_xLbTreeDocument->show();
                        bHideTreeDocument = false;
                    }
                }
            }
            catch (...)
            {
            }
        }

        if (bHideTreeDocument)
            m_xLbTreeDocument->hide();

    }
    else
        m_xLbTreeDocument->hide();
}

presentation::ClickAction SdTPAction::GetActualClickAction()
{
    presentation::ClickAction eCA = presentation::ClickAction_NONE;
    int nPos = m_xLbAction->get_active();
    if (nPos != -1 && static_cast<size_t>(nPos) < maCurrentActions.size())
        eCA = maCurrentActions[ nPos ];
    return eCA;
}

void SdTPAction::SetActualClickAction( presentation::ClickAction eCA )
{
    std::vector<css::presentation::ClickAction>::const_iterator pIter =
            std::find(maCurrentActions.begin(),maCurrentActions.end(),eCA);

    if ( pIter != maCurrentActions.end() )
        m_xLbAction->set_active(pIter-maCurrentActions.begin());
}

void SdTPAction::SetEditText( OUString const & rStr )
{
    presentation::ClickAction   eCA = GetActualClickAction();
    OUString                    aText(rStr);

    // possibly convert URI back to system path
    switch( eCA )
    {
        case presentation::ClickAction_DOCUMENT:
            if( comphelper::string::getTokenCount(rStr, DOCUMENT_TOKEN) == 2 )
                aText = rStr.getToken( 0, DOCUMENT_TOKEN );

            [[fallthrough]];
        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_PROGRAM:
            {
                INetURLObject aURL( aText );

                // try to convert to system path
                OUString aTmpStr(aURL.getFSysPath(FSysStyle::Detect));

                if( !aTmpStr.isEmpty() )
                    aText = aTmpStr;    // was a system path
            }
            break;
        default:
            break;
    }

    // set the string to the corresponding control
    switch( eCA )
    {
        case presentation::ClickAction_SOUND:
            m_xEdtSound->set_text(aText );
            break;
        case presentation::ClickAction_VERB:
            {
                ::std::vector< long >::iterator aFound( ::std::find( aVerbVector.begin(), aVerbVector.end(), rStr.toInt32() ) );
                if( aFound != aVerbVector.end() )
                    m_xLbOLEAction->select(aFound - aVerbVector.begin());
            }
            break;
        case presentation::ClickAction_PROGRAM:
            m_xEdtProgram->set_text( aText );
            break;
        case presentation::ClickAction_MACRO:
            m_xEdtMacro->set_text( aText );
            break;
        case presentation::ClickAction_DOCUMENT:
            m_xEdtDocument->set_text( aText );
            break;
        case presentation::ClickAction_BOOKMARK:
            m_xEdtBookmark->set_text( aText );
            break;
        default:
            break;
    }
}

OUString SdTPAction::GetEditText( bool bFullDocDestination )
{
    OUString aStr;
    presentation::ClickAction eCA = GetActualClickAction();

    switch( eCA )
    {
        case presentation::ClickAction_SOUND:
            aStr = m_xEdtSound->get_text();
            break;
        case presentation::ClickAction_VERB:
            {
                const int nPos = m_xLbOLEAction->get_selected_index();
                if (nPos != -1 && static_cast<size_t>(nPos) < aVerbVector.size() )
                    aStr = OUString::number( aVerbVector[ nPos ] );
                return aStr;
            }
        case presentation::ClickAction_DOCUMENT:
            aStr = m_xEdtDocument->get_text();
            break;

        case presentation::ClickAction_PROGRAM:
            aStr = m_xEdtProgram->get_text();
            break;

        case presentation::ClickAction_MACRO:
        {
            return m_xEdtMacro->get_text();
        }

        case presentation::ClickAction_BOOKMARK:
            return m_xEdtBookmark->get_text();

        default:
            break;
    }

    // validate file URI
    INetURLObject aURL( aStr );
    OUString aBaseURL;
    if( mpDoc && mpDoc->GetDocSh() && mpDoc->GetDocSh()->GetMedium() )
        aBaseURL = mpDoc->GetDocSh()->GetMedium()->GetBaseURL();

    if( !aStr.isEmpty() && aURL.GetProtocol() == INetProtocol::NotValid )
        aURL = INetURLObject( ::URIHelper::SmartRel2Abs( INetURLObject(aBaseURL), aStr, URIHelper::GetMaybeFileHdl() ) );

    // get adjusted file name
    aStr = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if( bFullDocDestination &&
        eCA == presentation::ClickAction_DOCUMENT &&
        m_xLbTreeDocument->get_visible() &&
        m_xLbTreeDocument->get_selected() )
    {
        OUString aTmpStr( m_xLbTreeDocument->get_selected_text() );
        if( !aTmpStr.isEmpty() )
        {
            aStr += OUStringLiteral1(DOCUMENT_TOKEN) + aTmpStr;
        }
    }

    return aStr;
}

const char* SdTPAction::GetClickActionSdResId( presentation::ClickAction eCA )
{
    switch( eCA )
    {
        case presentation::ClickAction_NONE:             return STR_CLICK_ACTION_NONE;
        case presentation::ClickAction_PREVPAGE:         return STR_CLICK_ACTION_PREVPAGE;
        case presentation::ClickAction_NEXTPAGE:         return STR_CLICK_ACTION_NEXTPAGE;
        case presentation::ClickAction_FIRSTPAGE:        return STR_CLICK_ACTION_FIRSTPAGE;
        case presentation::ClickAction_LASTPAGE:         return STR_CLICK_ACTION_LASTPAGE;
        case presentation::ClickAction_BOOKMARK:         return STR_CLICK_ACTION_BOOKMARK;
        case presentation::ClickAction_DOCUMENT:         return STR_CLICK_ACTION_DOCUMENT;
        case presentation::ClickAction_PROGRAM:          return STR_CLICK_ACTION_PROGRAM;
        case presentation::ClickAction_MACRO:            return STR_CLICK_ACTION_MACRO;
        case presentation::ClickAction_SOUND:            return STR_CLICK_ACTION_SOUND;
        case presentation::ClickAction_VERB:             return STR_CLICK_ACTION_VERB;
        case presentation::ClickAction_STOPPRESENTATION: return STR_CLICK_ACTION_STOPPRESENTATION;
        default: OSL_FAIL( "No StringResource for ClickAction available!" );
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
