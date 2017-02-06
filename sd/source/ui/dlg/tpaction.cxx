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
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/embed/NeedsRunningStateException.hpp>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/embed/VerbAttributes.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include "sdattr.hxx"
#include <sfx2/sfxresid.hxx>

#include <vcl/waitobj.hxx>
#include <osl/file.hxx>
#include <sfx2/app.hxx>
#include <unotools/pathoptions.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>
#include <unotools/localfilehelper.hxx>
#include <svl/aeitem.hxx>
#include <editeng/colritem.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/docfile.hxx>
#include <sot/storage.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <svx/xtable.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svx/drawitem.hxx>
#include "View.hxx"
#include "sdresid.hxx"
#include "tpaction.hxx"
#include "strmname.h"
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "filedlg.hxx"

#include <algorithm>

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

#define DOCUMENT_TOKEN '#'

/**
 * Constructor of the Tab dialog: appends the pages to the dialog
 */
SdActionDlg::SdActionDlg (
    vcl::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView )
    : SfxSingleTabDialog(pParent, *pAttr, "InteractionDialog",
        "modules/simpress/ui/interactiondialog.ui")
    , rOutAttrs(*pAttr)
{
    // FreeResource();
    VclPtr<SfxTabPage> pNewPage = SdTPAction::Create(get_content_area(), rOutAttrs);
    assert(pNewPage); //Unable to create page

    // formerly in PageCreated
    static_cast<SdTPAction*>( pNewPage.get() )->SetView( pView );
    static_cast<SdTPAction*>( pNewPage.get() )->Construct();

    SetTabPage( pNewPage );
}

/**
 *  Action-TabPage
 */
SdTPAction::SdTPAction(vcl::Window* pWindow, const SfxItemSet& rInAttrs)
    : SfxTabPage(pWindow, "InteractionPage",
        "modules/simpress/ui/interactionpage.ui", &rInAttrs)
    , mpView(nullptr)
    , mpDoc(nullptr)
    , bTreeUpdated(false)
{
    get(m_pLbAction, "listbox");
    get(m_pFtTree, "fttree");
    get(m_pLbTree, "tree");
    get(m_pLbTreeDocument, "treedoc");
    get(m_pLbOLEAction, "oleaction");
    get(m_pFrame, "frame");
    get(m_pEdtSound, "sound");
    get(m_pEdtBookmark, "bookmark");
    get(m_pEdtDocument, "document");
    get(m_pEdtProgram, "program");
    get(m_pEdtMacro, "macro");
    get(m_pBtnSearch, "browse");
    get(m_pBtnSeek, "find");

    m_pLbOLEAction->set_width_request(m_pLbOLEAction->approximate_char_width() * 52);
    m_pLbOLEAction->set_height_request(m_pLbOLEAction->GetTextHeight() * 12);

    m_pBtnSearch->SetClickHdl( LINK( this, SdTPAction, ClickSearchHdl ) );
    m_pBtnSeek->SetClickHdl( LINK( this, SdTPAction, ClickSearchHdl ) );

    // this page needs ExchangeSupport
    SetExchangeSupport();

    m_pLbAction->SetSelectHdl( LINK( this, SdTPAction, ClickActionHdl ) );
    m_pLbTree->SetSelectHdl( LINK( this, SdTPAction, SelectTreeHdl ) );
    m_pEdtDocument->SetLoseFocusHdl( LINK( this, SdTPAction, CheckFileHdl ) );
    m_pEdtMacro->SetLoseFocusHdl( LINK( this, SdTPAction, CheckFileHdl ) );

    //Lock to initial max size
    Size aSize(get_preferred_size());
    set_width_request(aSize.Width());
    set_height_request(aSize.Height());

    ClickActionHdl( *m_pLbAction );
}

SdTPAction::~SdTPAction()
{
    disposeOnce();
}

void SdTPAction::dispose()
{
    m_pLbAction.clear();
    m_pFtTree.clear();
    m_pLbTree.clear();
    m_pLbTreeDocument.clear();
    m_pLbOLEAction.clear();
    m_pFrame.clear();
    m_pEdtSound.clear();
    m_pEdtBookmark.clear();
    m_pEdtDocument.clear();
    m_pEdtProgram.clear();
    m_pEdtMacro.clear();
    m_pBtnSearch.clear();
    m_pBtnSeek.clear();
    SfxTabPage::dispose();
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
        m_pLbTree->SetViewFrame( pFrame );
        m_pLbTreeDocument->SetViewFrame( pFrame );

        SvxColorListItem aItem( *static_cast<const SvxColorListItem*>( pDocSh->GetItem( SID_COLOR_TABLE ) ) );
        pColList = aItem.GetColorList();
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
        m_pLbOLEAction->InsertEntry( MnemonicGenerator::EraseAllMnemonicChars( SD_RESSTR( STR_EDIT_OBJ ) ) );
    }
    else if( pOleObj )
    {
        uno::Reference < embed::XEmbeddedObject > xObj = pOleObj->GetObjRef();
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
                    m_pLbOLEAction->InsertEntry( MnemonicGenerator::EraseAllMnemonicChars( aTmp ) );
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
    if( bOLEAction && m_pLbOLEAction->GetEntryCount() )
        maCurrentActions.push_back( presentation::ClickAction_VERB );
    maCurrentActions.push_back( presentation::ClickAction_PROGRAM );
    maCurrentActions.push_back( presentation::ClickAction_MACRO );
    maCurrentActions.push_back( presentation::ClickAction_STOPPRESENTATION );

    // fill Action-Listbox
    for (presentation::ClickAction & rAction : maCurrentActions)
    {
        sal_uInt16 nRId = GetClickActionSdResId( rAction );
        m_pLbAction->InsertEntry( SD_RESSTR( nRId ) );
    }

}

bool SdTPAction::FillItemSet( SfxItemSet* rAttrs )
{
    bool bModified = false;
    presentation::ClickAction eCA = presentation::ClickAction_NONE;

    if( m_pLbAction->GetSelectEntryCount() )
        eCA = GetActualClickAction();

    if( m_pLbAction->IsValueChangedFromSaved() )
    {
        rAttrs->Put( SfxAllEnumItem( ATTR_ACTION, (sal_uInt16)eCA ) );
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

    // m_pLbAction
    if( rAttrs->GetItemState( ATTR_ACTION ) != SfxItemState::DONTCARE )
    {
        eCA = (presentation::ClickAction) static_cast<const SfxAllEnumItem&>( rAttrs->
                    Get( ATTR_ACTION ) ).GetValue();
        SetActualClickAction( eCA );
    }
    else
        m_pLbAction->SetNoSelection();

    // m_pEdtSound
    if( rAttrs->GetItemState( ATTR_ACTION_FILENAME ) != SfxItemState::DONTCARE )
    {
            aFileName = static_cast<const SfxStringItem&>( rAttrs->Get( ATTR_ACTION_FILENAME ) ).GetValue();
            SetEditText( aFileName );
    }

    switch( eCA )
    {
        case presentation::ClickAction_BOOKMARK:
        {
            if( !m_pLbTree->SelectEntry( aFileName ) )
                m_pLbTree->SelectAll( false );
        }
        break;

        case presentation::ClickAction_DOCUMENT:
        {
            if( comphelper::string::getTokenCount(aFileName, DOCUMENT_TOKEN) == 2 )
                m_pLbTreeDocument->SelectEntry( aFileName.getToken( 1, DOCUMENT_TOKEN ) );
        }
        break;

        default:
        break;
    }
    ClickActionHdl( *m_pLbAction );

    m_pLbAction->SaveValue();
    m_pEdtSound->SaveValue();
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

VclPtr<SfxTabPage> SdTPAction::Create( vcl::Window* pWindow,
                                       const SfxItemSet& rAttrs )
{
    return VclPtr<SdTPAction>::Create( pWindow, rAttrs );
}

void SdTPAction::UpdateTree()
{
    if( !bTreeUpdated && mpDoc && mpDoc->GetDocSh() && mpDoc->GetDocSh()->GetMedium() )
    {
        //m_pLbTree->Clear();
        m_pLbTree->Fill( mpDoc, true, mpDoc->GetDocSh()->GetMedium()->GetName() );
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
        m_pLbTree->SelectEntry( GetEditText() );
    }
    else
    {
        OUString aFile( GetEditText() );

        if (bSound)
        {
            SdOpenSoundFileDialog   aFileDialog;

            if( aFile.isEmpty() )
                aFile = SvtPathOptions().GetWorkPath();

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
            OUString aScriptURL = SfxApplication::ChooseScript();

            if ( !aScriptURL.isEmpty() )
            {
                SetEditText( aScriptURL );
            }
        }
        else
        {
            sfx2::FileDialogHelper aFileDialog(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION);

            if (bDocument && aFile.isEmpty())
                aFile = SvtPathOptions().GetWorkPath();

            aFileDialog.SetDisplayDirectory( aFile );

            // The following is a workaround for #i4306#:
            // The addition of the implicitly existing "all files"
            // filter makes the (Windows system) open file dialog follow
            // links on the desktop to directories.
            aFileDialog.AddFilter (
                SFX2_RESSTR(STR_SFX_FILTERNAME_ALL),
                "*.*");

            if( aFileDialog.Execute() == ERRCODE_NONE )
            {
                aFile = aFileDialog.GetPath();
                SetEditText( aFile );
            }
            if( bDocument )
                CheckFileHdl( *m_pEdtDocument );
        }
    }
}

IMPL_LINK_NOARG(SdTPAction, ClickSearchHdl, Button*, void)
{
    OpenFileDialog();
}

IMPL_LINK_NOARG(SdTPAction, ClickActionHdl, ListBox&, void)
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
            m_pFtTree->Hide();
            m_pLbTree->Hide();
            m_pLbTreeDocument->Hide();
            m_pLbOLEAction->Hide();

            m_pFrame->Hide();
            m_pEdtSound->Hide();
            m_pEdtBookmark->Hide();
            m_pEdtDocument->Hide();
            m_pEdtProgram->Hide();
            m_pEdtMacro->Hide();
            m_pBtnSearch->Hide();
            m_pBtnSeek->Hide();
            break;

        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_PROGRAM:
        case presentation::ClickAction_MACRO:
            m_pFtTree->Hide();
            m_pLbTree->Hide();
            m_pLbTreeDocument->Hide();
            m_pLbOLEAction->Hide();

            m_pEdtDocument->Hide();

            if( eCA == presentation::ClickAction_MACRO )
            {
                m_pEdtSound->Hide();
                m_pEdtProgram->Hide();
            }
            else if( eCA == presentation::ClickAction_PROGRAM )
            {
                m_pEdtSound->Hide();
                m_pEdtMacro->Hide();
            }
            else if( eCA == presentation::ClickAction_SOUND )
            {
                m_pEdtProgram->Hide();
                m_pEdtMacro->Hide();
            }

            m_pBtnSeek->Hide();
            break;

        case presentation::ClickAction_DOCUMENT:
            m_pLbTree->Hide();
            m_pLbOLEAction->Hide();

            m_pEdtSound->Hide();
            m_pEdtProgram->Hide();
            m_pEdtMacro->Hide();
            m_pEdtBookmark->Hide();
            m_pBtnSeek->Hide();
            break;

        case presentation::ClickAction_BOOKMARK:
            m_pLbTreeDocument->Hide();
            m_pLbOLEAction->Hide();
            m_pEdtSound->Hide();
            m_pEdtDocument->Hide();
            m_pEdtProgram->Hide();
            m_pEdtMacro->Hide();
            m_pBtnSearch->Hide();
            break;

        case presentation::ClickAction_VERB:
            m_pLbTree->Hide();
            m_pEdtDocument->Hide();
            m_pEdtProgram->Hide();
            m_pEdtBookmark->Hide();
            m_pEdtMacro->Hide();
            m_pBtnSearch->Hide();
            m_pFrame->Hide();
            m_pEdtSound->Hide();
            m_pBtnSeek->Hide();
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
            m_pFrame->Show();
            m_pEdtSound->Show();
            m_pEdtSound->Enable();
            m_pBtnSearch->Show();
            m_pBtnSearch->Enable();
            m_pFrame->set_label( SD_RESSTR( STR_EFFECTDLG_SOUND ) );
            break;

        case presentation::ClickAction_PROGRAM:
        case presentation::ClickAction_MACRO:
            m_pFrame->Show();
            m_pBtnSearch->Show();
            m_pBtnSearch->Enable();
            if( eCA == presentation::ClickAction_MACRO )
            {
                m_pEdtMacro->Show();
                m_pFrame->set_label( SD_RESSTR( STR_EFFECTDLG_MACRO ) );
            }
            else
            {
                m_pEdtProgram->Show();
                m_pFrame->set_label( SD_RESSTR( STR_EFFECTDLG_PROGRAM ) );
            }
            break;

        case presentation::ClickAction_DOCUMENT:
            m_pFtTree->Show();
            m_pLbTreeDocument->Show();

            m_pFrame->Show();
            m_pEdtDocument->Show();
            m_pBtnSearch->Show();
            m_pBtnSearch->Enable();

            m_pFtTree->SetText( SD_RESSTR( STR_EFFECTDLG_JUMP ) );
            m_pFrame->set_label( SD_RESSTR( STR_EFFECTDLG_DOCUMENT ) );

            CheckFileHdl( *m_pEdtDocument );
            break;

        case presentation::ClickAction_VERB:
            m_pFtTree->Show();
            m_pLbOLEAction->Show();

            m_pFtTree->SetText( SD_RESSTR( STR_EFFECTDLG_ACTION ) );
            break;

        case presentation::ClickAction_BOOKMARK:
            UpdateTree();

            m_pFtTree->Show();
            m_pLbTree->Show();

            m_pFrame->Show();
            m_pEdtBookmark->Show();
            m_pBtnSeek->Show();

            m_pFtTree->SetText( SD_RESSTR( STR_EFFECTDLG_JUMP ) );
            m_pFrame->set_label( SD_RESSTR( STR_EFFECTDLG_PAGE_OBJECT ) );
            break;
        default:
            break;
    }
}

IMPL_LINK_NOARG(SdTPAction, SelectTreeHdl, SvTreeListBox*, void)
{
    m_pEdtBookmark->SetText( m_pLbTree->GetSelectEntry() );
}

IMPL_LINK_NOARG(SdTPAction, CheckFileHdl, Control&, void)
{
    OUString aFile( GetEditText() );

    if( aFile != aLastFile )
    {
        // check if it is a valid draw file
        SfxMedium aMedium( aFile,
                    StreamMode::READ | StreamMode::NOCREATE );

        if( aMedium.IsStorage() )
        {
            WaitObject aWait( GetParentDialog() );

            // is it a draw file?
            // open with READ, otherwise the Storages might write into the file!
            uno::Reference < embed::XStorage > xStorage = aMedium.GetStorage();
            DBG_ASSERT( xStorage.is(), "No storage!" );

            uno::Reference < container::XNameAccess > xAccess( xStorage, uno::UNO_QUERY );
            if( xAccess.is() &&
                ( xAccess->hasByName( pStarDrawXMLContent ) ||
                xAccess->hasByName( pStarDrawOldXMLContent ) ) )
            {
                SdDrawDocument* pBookmarkDoc = mpDoc->OpenBookmarkDoc( aFile );
                if( pBookmarkDoc )
                {
                    aLastFile = aFile;

                    m_pLbTreeDocument->Clear();
                    m_pLbTreeDocument->Fill( pBookmarkDoc, true, aFile );
                    mpDoc->CloseBookmarkDoc();
                    m_pLbTreeDocument->Show();
                }
                else
                    m_pLbTreeDocument->Hide();
            }
            else
                m_pLbTreeDocument->Hide();

        }
        else
            m_pLbTreeDocument->Hide();
    }
}

presentation::ClickAction SdTPAction::GetActualClickAction()
{
    presentation::ClickAction eCA = presentation::ClickAction_NONE;
    sal_Int32 nPos = m_pLbAction->GetSelectEntryPos();

    if (nPos != LISTBOX_ENTRY_NOTFOUND && static_cast<size_t>(nPos) < maCurrentActions.size())
        eCA = maCurrentActions[ nPos ];
    return eCA;
}

void SdTPAction::SetActualClickAction( presentation::ClickAction eCA )
{
    std::vector<css::presentation::ClickAction>::const_iterator pIter =
            std::find(maCurrentActions.begin(),maCurrentActions.end(),eCA);

    if ( pIter != maCurrentActions.end() )
        m_pLbAction->SelectEntryPos( pIter-maCurrentActions.begin() );
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

            SAL_FALLTHROUGH;
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
            m_pEdtSound->SetText(aText );
            break;
        case presentation::ClickAction_VERB:
            {
                ::std::vector< long >::iterator aFound( ::std::find( aVerbVector.begin(), aVerbVector.end(), rStr.toInt32() ) );
                if( aFound != aVerbVector.end() )
                    m_pLbOLEAction->SelectEntryPos( static_cast< short >( aFound - aVerbVector.begin() ) );
            }
            break;
        case presentation::ClickAction_PROGRAM:
            m_pEdtProgram->SetText( aText );
            break;
        case presentation::ClickAction_MACRO:
        {
            m_pEdtMacro->SetText( aText );
        }
            break;
        case presentation::ClickAction_DOCUMENT:
            m_pEdtDocument->SetText( aText );
            break;
        case presentation::ClickAction_BOOKMARK:
            m_pEdtBookmark->SetText( aText );
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
            aStr = m_pEdtSound->GetText();
            break;
        case presentation::ClickAction_VERB:
            {
                const sal_Int32 nPos = m_pLbOLEAction->GetSelectEntryPos();
                if( static_cast<size_t>(nPos) < aVerbVector.size() )
                    aStr = OUString::number( aVerbVector[ nPos ] );
                return aStr;
            }
        case presentation::ClickAction_DOCUMENT:
            aStr = m_pEdtDocument->GetText();
            break;

        case presentation::ClickAction_PROGRAM:
            aStr = m_pEdtProgram->GetText();
            break;

        case presentation::ClickAction_MACRO:
        {
            return m_pEdtMacro->GetText();
        }

        case presentation::ClickAction_BOOKMARK:
            return m_pEdtBookmark->GetText();

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
        m_pLbTreeDocument->Control::IsVisible() &&
        m_pLbTreeDocument->GetSelectionCount() > 0 )
    {
        OUString aTmpStr( m_pLbTreeDocument->GetSelectEntry() );
        if( !aTmpStr.isEmpty() )
        {
            aStr += OUStringLiteral1(DOCUMENT_TOKEN) + aTmpStr;
        }
    }

    return aStr;
}

sal_uInt16 SdTPAction::GetClickActionSdResId( presentation::ClickAction eCA )
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
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
