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
#include "tpaction.hrc"
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

#define DOCUMENT_TOKEN (sal_Unicode('#'))

/**
 * Constructor of the Tab dialog: appends the pages to the dialog
 */
SdActionDlg::SdActionDlg (
    ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView ) :
        SfxNoLayoutSingleTabDialog  ( pParent, *pAttr, TP_ANIMATION_ACTION ),
        rOutAttrs           ( *pAttr )
{
    // FreeResource();
    SfxTabPage* pNewPage = SdTPAction::Create( this, rOutAttrs );
    DBG_ASSERT( pNewPage, "Unable to create page");

    // formerly in PageCreated
    ( (SdTPAction*) pNewPage )->SetView( pView );
    ( (SdTPAction*) pNewPage )->Construct();

    SetTabPage( pNewPage );

    String aStr( pNewPage->GetText() );
    if( aStr.Len() )
        SetText( aStr );
}


/**
 *  Action-TabPage
 */
SdTPAction::SdTPAction( Window* pWindow, const SfxItemSet& rInAttrs ) :
        SfxTabPage      ( pWindow, SdResId( TP_ANIMATION ), rInAttrs ),

        aFtAction       ( this, SdResId( FT_ACTION ) ),
        aLbAction       ( this, SdResId( LB_ACTION ) ),
        aFtTree         ( this, SdResId( FT_TREE ) ),
        aLbTree         ( this, SdResId( LB_TREE ) ),
        aLbTreeDocument ( this, SdResId( LB_TREE_DOCUMENT ) ),
        aLbOLEAction    ( this, SdResId( LB_OLE_ACTION ) ),
        aFlSeparator    ( this, SdResId( FL_SEPARATOR ) ),
        aEdtSound       ( this, SdResId( EDT_SOUND ) ),
        aEdtBookmark    ( this, SdResId( EDT_BOOKMARK ) ),
        aEdtDocument    ( this, SdResId( EDT_DOCUMENT ) ),
        aEdtProgram     ( this, SdResId( EDT_PROGRAM ) ),
        aEdtMacro       ( this, SdResId( EDT_MACRO ) ),
        aBtnSearch      ( this, SdResId( BTN_SEARCH ) ),
        aBtnSeek        ( this, SdResId( BTN_SEEK ) ),

        rOutAttrs       ( rInAttrs ),
        mpView          ( NULL ),
        mpDoc           ( NULL ),
        bTreeUpdated    ( sal_False )
{
    aEdtSound.SetAccessibleName(String(SdResId(STR_PATHNAME)));
    aBtnSeek.SetAccessibleRelationMemberOf( &aFlSeparator );

    FreeResource();

    aBtnSearch.SetClickHdl( LINK( this, SdTPAction, ClickSearchHdl ) );
    aBtnSeek.SetClickHdl( LINK( this, SdTPAction, ClickSearchHdl ) );

    // this page needs ExchangeSupport
    SetExchangeSupport();

    aLbAction.SetSelectHdl( LINK( this, SdTPAction, ClickActionHdl ) );
    aLbTree.SetSelectHdl( LINK( this, SdTPAction, SelectTreeHdl ) );
    aEdtDocument.SetLoseFocusHdl( LINK( this, SdTPAction, CheckFileHdl ) );
    aEdtMacro.SetLoseFocusHdl( LINK( this, SdTPAction, CheckFileHdl ) );

    // enable controls
    aFtAction.Show();
    aLbAction.Show();

    ClickActionHdl( this );
}

// -----------------------------------------------------------------------

SdTPAction::~SdTPAction()
{
}

// -----------------------------------------------------------------------

void SdTPAction::SetView( const ::sd::View* pSdView )
{
    mpView = pSdView;

    // get ColorTable and fill ListBox
    ::sd::DrawDocShell* pDocSh = static_cast<const ::sd::View*>(mpView)->GetDocSh();
    if( pDocSh && pDocSh->GetViewShell() )
    {
        mpDoc = pDocSh->GetDoc();
        SfxViewFrame* pFrame = pDocSh->GetViewShell()->GetViewFrame();
        aLbTree.SetViewFrame( pFrame );
        aLbTreeDocument.SetViewFrame( pFrame );

        SvxColorListItem aItem( *(const SvxColorListItem*)( pDocSh->GetItem( SID_COLOR_TABLE ) ) );
        pColList = aItem.GetColorList();
        DBG_ASSERT( pColList.is(), "No color table available!" );
    }
    else
    {
        OSL_FAIL("sd::SdTPAction::SetView(), no docshell or viewshell?");
    }
}

// -----------------------------------------------------------------------

void SdTPAction::Construct()
{
    // fill OLE-Actionlistbox
    SdrOle2Obj* pOleObj = NULL;
    SdrGrafObj* pGrafObj = NULL;
    sal_Bool        bOLEAction = sal_False;

    if ( mpView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
        SdrObject* pObj;

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            pObj = pMark->GetMarkedSdrObj();

            sal_uInt32 nInv = pObj->GetObjInventor();
            sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv == SdrInventor && nSdrObjKind == OBJ_OLE2)
            {
                pOleObj = (SdrOle2Obj*) pObj;
            }
            else if (nInv == SdrInventor && nSdrObjKind == OBJ_GRAF)
            {
                pGrafObj = (SdrGrafObj*) pObj;
            }
        }
    }
    if( pGrafObj )
    {
        bOLEAction = sal_True;

        aVerbVector.push_back( 0 );
        aLbOLEAction.InsertEntry( MnemonicGenerator::EraseAllMnemonicChars( String( SdResId( STR_EDIT_OBJ ) ) ) );
    }
    else if( pOleObj )
    {
        uno::Reference < embed::XEmbeddedObject > xObj = pOleObj->GetObjRef();
        if ( xObj.is() )
        {
            bOLEAction = sal_True;
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
                    String aTmp( aVerb.VerbName );
                    aVerbVector.push_back( aVerb.VerbID );
                    aLbOLEAction.InsertEntry( MnemonicGenerator::EraseAllMnemonicChars( aTmp ) );
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
    if( bOLEAction && aLbOLEAction.GetEntryCount() )
        maCurrentActions.push_back( presentation::ClickAction_VERB );
    maCurrentActions.push_back( presentation::ClickAction_PROGRAM );
    maCurrentActions.push_back( presentation::ClickAction_MACRO );
    maCurrentActions.push_back( presentation::ClickAction_STOPPRESENTATION );

    // fill Action-Listbox
    for (size_t nAction = 0, n = maCurrentActions.size(); nAction < n; nAction++)
    {
        sal_uInt16 nRId = GetClickActionSdResId( maCurrentActions[ nAction ] );
        aLbAction.InsertEntry( String( SdResId( nRId ) ) );
    }

}

// -----------------------------------------------------------------------

sal_Bool SdTPAction::FillItemSet( SfxItemSet& rAttrs )
{
    sal_Bool bModified = sal_False;
    presentation::ClickAction eCA = presentation::ClickAction_NONE;

    if( aLbAction.GetSelectEntryCount() )
        eCA = GetActualClickAction();

    if( aLbAction.GetSavedValue() != aLbAction.GetSelectEntryPos() )
    {
        rAttrs.Put( SfxAllEnumItem( ATTR_ACTION, (sal_uInt16)eCA ) );
        bModified = sal_True;
    }
    else
        rAttrs.InvalidateItem( ATTR_ACTION );

    String aFileName = GetEditText( sal_True );
    if( aFileName.Len() == 0 )
        rAttrs.InvalidateItem( ATTR_ACTION_FILENAME );
    else
    {
        if( mpDoc && mpDoc->GetDocSh() && mpDoc->GetDocSh()->GetMedium() )
        {
            String aBaseURL = mpDoc->GetDocSh()->GetMedium()->GetBaseURL();
            if( eCA == presentation::ClickAction_SOUND ||
                eCA == presentation::ClickAction_DOCUMENT ||
                eCA == presentation::ClickAction_PROGRAM )
                aFileName = ::URIHelper::SmartRel2Abs( INetURLObject(aBaseURL), aFileName, URIHelper::GetMaybeFileHdl(), true, false,
                                                        INetURLObject::WAS_ENCODED,
                                                        INetURLObject::DECODE_UNAMBIGUOUS );

            rAttrs.Put( SfxStringItem( ATTR_ACTION_FILENAME, aFileName ) );
            bModified = sal_True;
        }
        else
        {
            OSL_FAIL("sd::SdTPAction::FillItemSet(), I need a medium!");
        }
    }

    return( bModified );
}

//------------------------------------------------------------------------

void SdTPAction::Reset( const SfxItemSet& rAttrs )
{
    presentation::ClickAction eCA = presentation::ClickAction_NONE;
    String      aFileName;

    // aLbAction
    if( rAttrs.GetItemState( ATTR_ACTION ) != SFX_ITEM_DONTCARE )
    {
        eCA = (presentation::ClickAction) ( ( const SfxAllEnumItem& ) rAttrs.
                    Get( ATTR_ACTION ) ).GetValue();
        SetActualClickAction( eCA );
    }
    else
        aLbAction.SetNoSelection();

    // aEdtSound
    if( rAttrs.GetItemState( ATTR_ACTION_FILENAME ) != SFX_ITEM_DONTCARE )
    {
            aFileName = ( ( const SfxStringItem& ) rAttrs.Get( ATTR_ACTION_FILENAME ) ).GetValue();
            SetEditText( aFileName );
    }

    switch( eCA )
    {
        case presentation::ClickAction_BOOKMARK:
        {
            if( !aLbTree.SelectEntry( aFileName ) )
                aLbTree.SelectAll( sal_False );
        }
        break;

        case presentation::ClickAction_DOCUMENT:
        {
            if( comphelper::string::getTokenCount(aFileName, DOCUMENT_TOKEN) == 2 )
                aLbTreeDocument.SelectEntry( aFileName.GetToken( 1, DOCUMENT_TOKEN ) );
        }
        break;

        default:
        break;
    }
    ClickActionHdl( this );

    aLbAction.SaveValue();
    aEdtSound.SaveValue();
}

// -----------------------------------------------------------------------

void SdTPAction::ActivatePage( const SfxItemSet& )
{
}

// -----------------------------------------------------------------------

int SdTPAction::DeactivatePage( SfxItemSet* pPageSet )
{
    if( pPageSet )
        FillItemSet( *pPageSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

SfxTabPage* SdTPAction::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTPAction( pWindow, rAttrs ) );
}

//------------------------------------------------------------------------

void SdTPAction::UpdateTree()
{
    if( !bTreeUpdated && mpDoc && mpDoc->GetDocSh() && mpDoc->GetDocSh()->GetMedium() )
    {
        //aLbTree.Clear();
        aLbTree.Fill( mpDoc, sal_True, mpDoc->GetDocSh()->GetMedium()->GetName() );
        bTreeUpdated = sal_True;
    }
}

//------------------------------------------------------------------------

void SdTPAction::OpenFileDialog()
{
    // Soundpreview only for interaction with sound
    presentation::ClickAction eCA = GetActualClickAction();
    sal_Bool bSound = ( eCA == presentation::ClickAction_SOUND );
    sal_Bool bPage = ( eCA == presentation::ClickAction_BOOKMARK );
    sal_Bool bDocument = ( eCA == presentation::ClickAction_DOCUMENT ||
                       eCA == presentation::ClickAction_PROGRAM );
    sal_Bool bMacro = ( eCA == presentation::ClickAction_MACRO );

    if( bPage )
    {
        // search in the TreeLB for the specified object
        aLbTree.SelectEntry( GetEditText() );
    }
    else
    {
        String aFile( GetEditText() );

        if (bSound)
        {
            SdOpenSoundFileDialog   aFileDialog;

            if( !aFile.Len() )
                aFile = SvtPathOptions().GetGraphicPath();

            aFileDialog.SetPath( aFile );

            if( aFileDialog.Execute() == ERRCODE_NONE )
            {
                aFile = aFileDialog.GetPath();
                SetEditText( aFile );
            }
        }
        else if (bMacro)
        {
            Window* pOldWin = Application::GetDefDialogParent();
            Application::SetDefDialogParent( this );

            // choose macro dialog
            OUString aScriptURL = SfxApplication::ChooseScript();

            if ( !aScriptURL.isEmpty() )
            {
                SetEditText( aScriptURL );
            }

            Application::SetDefDialogParent( pOldWin );
        }
        else
        {
            sfx2::FileDialogHelper aFileDialog(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION, 0);

            if (bDocument && !aFile.Len())
                aFile = SvtPathOptions().GetWorkPath();

            aFileDialog.SetDisplayDirectory( aFile );

            // The following is a workarround for #i4306#:
            // The addition of the implicitely existing "all files"
            // filter makes the (Windows system) open file dialog follow
            // links on the desktop to directories.
            aFileDialog.AddFilter (
                String (SfxResId (STR_SFX_FILTERNAME_ALL)),
                OUString("*.*"));


            if( aFileDialog.Execute() == ERRCODE_NONE )
            {
                aFile = aFileDialog.GetPath();
                SetEditText( aFile );
            }
            if( bDocument )
                CheckFileHdl( NULL );
        }
    }
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SdTPAction, ClickSearchHdl)
{
    OpenFileDialog();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SdTPAction, ClickActionHdl)
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
            aFtTree.Hide();
            aLbTree.Hide();
            aLbTreeDocument.Hide();
            aLbOLEAction.Hide();

            aFlSeparator.Hide();
            aEdtSound.Hide();
            aEdtBookmark.Hide();
            aEdtDocument.Hide();
            aEdtProgram.Hide();
            aEdtMacro.Hide();
            aBtnSearch.Hide();
            aBtnSeek.Hide();
            break;

        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_PROGRAM:
        case presentation::ClickAction_MACRO:
            aFtTree.Hide();
            aLbTree.Hide();
            aLbTreeDocument.Hide();
            aLbOLEAction.Hide();

            aEdtDocument.Hide();

            if( eCA == presentation::ClickAction_MACRO )
            {
                aEdtSound.Hide();
                aEdtProgram.Hide();
            }
            else if( eCA == presentation::ClickAction_PROGRAM )
            {
                aEdtSound.Hide();
                aEdtMacro.Hide();
            }
            else if( eCA == presentation::ClickAction_SOUND )
            {
                aEdtProgram.Hide();
                aEdtMacro.Hide();
            }

            aBtnSeek.Hide();
            break;


        case presentation::ClickAction_DOCUMENT:
            aLbTree.Hide();
            aLbOLEAction.Hide();

            aEdtSound.Hide();
            aEdtProgram.Hide();
            aEdtMacro.Hide();
            aEdtBookmark.Hide();
            aBtnSeek.Hide();
            break;

        case presentation::ClickAction_BOOKMARK:
            aLbTreeDocument.Hide();
            aLbOLEAction.Hide();
            aEdtSound.Hide();
            aEdtDocument.Hide();
            aEdtProgram.Hide();
            aEdtMacro.Hide();
            aBtnSearch.Hide();
            break;

        case presentation::ClickAction_VERB:
            aLbTree.Hide();
            aEdtDocument.Hide();
            aEdtProgram.Hide();
            aEdtBookmark.Hide();
            aEdtMacro.Hide();
            aBtnSearch.Hide();
            aFlSeparator.Hide();
            aEdtSound.Hide();
            aBtnSeek.Hide();
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
            aFlSeparator.Show();
            aEdtSound.Show();
            aEdtSound.Enable();
            aBtnSearch.Show();
            aBtnSearch.Enable();
            aFlSeparator.SetText( String( SdResId( STR_EFFECTDLG_SOUND ) ) );
            break;

        case presentation::ClickAction_PROGRAM:
        case presentation::ClickAction_MACRO:
            aFlSeparator.Show();
            aBtnSearch.Show();
            aBtnSearch.Enable();
            if( eCA == presentation::ClickAction_MACRO )
            {
                aEdtMacro.Show();
                aFlSeparator.SetText( String( SdResId( STR_EFFECTDLG_MACRO ) ) );
            }
            else
            {
                aEdtProgram.Show();
                aFlSeparator.SetText( String( SdResId( STR_EFFECTDLG_PROGRAM ) ) );
            }
            break;

        case presentation::ClickAction_DOCUMENT:
            aFtTree.Show();
            aLbTreeDocument.Show();

            aFlSeparator.Show();
            aEdtDocument.Show();
            aBtnSearch.Show();
            aBtnSearch.Enable();

            aFtTree.SetText( String( SdResId( STR_EFFECTDLG_JUMP ) ) );
            aFlSeparator.SetText( String( SdResId( STR_EFFECTDLG_DOCUMENT ) ) );

            CheckFileHdl( NULL );
            break;

        case presentation::ClickAction_VERB:
            aFtTree.Show();
            aLbOLEAction.Show();

            aFtTree.SetText( String( SdResId( STR_EFFECTDLG_ACTION ) ) );
            break;

        case presentation::ClickAction_BOOKMARK:
            UpdateTree();

            aFtTree.Show();
            aLbTree.Show();

            aFlSeparator.Show();
            aEdtBookmark.Show();
            aBtnSeek.Show();

            aFtTree.SetText( String( SdResId( STR_EFFECTDLG_JUMP ) ) );
            aFlSeparator.SetText( String( SdResId( STR_EFFECTDLG_PAGE_OBJECT ) ) );
            break;
        default:
            break;
    }

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SdTPAction, SelectTreeHdl)
{
    aEdtBookmark.SetText( aLbTree.GetSelectEntry() );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SdTPAction, CheckFileHdl)
{
    String aFile( GetEditText() );

    if( aFile != aLastFile )
    {
        // check if it is a valid draw file
        SfxMedium aMedium( aFile,
                    STREAM_READ | STREAM_NOCREATE );

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

                    aLbTreeDocument.Clear();
                    aLbTreeDocument.Fill( pBookmarkDoc, sal_True, aFile );
                    mpDoc->CloseBookmarkDoc();
                    aLbTreeDocument.Show();
                }
                else
                    aLbTreeDocument.Hide();
            }
            else
                aLbTreeDocument.Hide();

        }
        else
            aLbTreeDocument.Hide();
    }

    return( 0L );
}

//------------------------------------------------------------------------

presentation::ClickAction SdTPAction::GetActualClickAction()
{
    presentation::ClickAction eCA = presentation::ClickAction_NONE;
    sal_uInt16 nPos = aLbAction.GetSelectEntryPos();

    if (nPos != LISTBOX_ENTRY_NOTFOUND && nPos < maCurrentActions.size())
        eCA = maCurrentActions[ nPos ];
    return( eCA );
}

//------------------------------------------------------------------------

void SdTPAction::SetActualClickAction( presentation::ClickAction eCA )
{
    std::vector<com::sun::star::presentation::ClickAction>::const_iterator pIter =
            std::find(maCurrentActions.begin(),maCurrentActions.end(),eCA);

    if ( pIter != maCurrentActions.end() )
        aLbAction.SelectEntryPos( pIter-maCurrentActions.begin() );
}

//------------------------------------------------------------------------

void SdTPAction::SetEditText( String const & rStr )
{
    presentation::ClickAction   eCA = GetActualClickAction();
    String                      aText(rStr);

    // possibly convert URI back to system path
    switch( eCA )
    {
        case presentation::ClickAction_DOCUMENT:
            if( comphelper::string::getTokenCount(rStr, DOCUMENT_TOKEN) == 2 )
                aText = rStr.GetToken( 0, DOCUMENT_TOKEN );

            // fallthrough inteded
        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_PROGRAM:
            {
                INetURLObject aURL( aText );

                // try to convert to system path
                String aTmpStr(aURL.getFSysPath(INetURLObject::FSYS_DETECT));

                if( aTmpStr.Len() )
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
            aEdtSound.SetText(aText );
            break;
        case presentation::ClickAction_VERB:
            {
                ::std::vector< long >::iterator aFound( ::std::find( aVerbVector.begin(), aVerbVector.end(), rStr.ToInt32() ) );
                if( aFound != aVerbVector.end() )
                    aLbOLEAction.SelectEntryPos( static_cast< short >( aFound - aVerbVector.begin() ) );
            }
            break;
        case presentation::ClickAction_PROGRAM:
            aEdtProgram.SetText( aText );
            break;
        case presentation::ClickAction_MACRO:
        {
            aEdtMacro.SetText( aText );
        }
            break;
        case presentation::ClickAction_DOCUMENT:
            aEdtDocument.SetText( aText );
            break;
        case presentation::ClickAction_BOOKMARK:
            aEdtBookmark.SetText( aText );
            break;
        default:
            break;
    }
}

//------------------------------------------------------------------------

String SdTPAction::GetEditText( sal_Bool bFullDocDestination )
{
    String aStr;
    presentation::ClickAction eCA = GetActualClickAction();

    switch( eCA )
    {
        case presentation::ClickAction_SOUND:
            aStr = aEdtSound.GetText();
            break;
        case presentation::ClickAction_VERB:
            {
                const sal_uInt16 nPos = aLbOLEAction.GetSelectEntryPos();
                if( nPos < aVerbVector.size() )
                    aStr = OUString::number( aVerbVector[ nPos ] );
                return aStr;
            }
        case presentation::ClickAction_DOCUMENT:
            aStr = aEdtDocument.GetText();
            break;

        case presentation::ClickAction_PROGRAM:
            aStr = aEdtProgram.GetText();
            break;

        case presentation::ClickAction_MACRO:
        {
            return aEdtMacro.GetText();
        }

        case presentation::ClickAction_BOOKMARK:
            return( aEdtBookmark.GetText() );

        default:
            break;
    }

    // validate file URI
    INetURLObject aURL( aStr );
    String aBaseURL;
    if( mpDoc && mpDoc->GetDocSh() && mpDoc->GetDocSh()->GetMedium() )
        aBaseURL = mpDoc->GetDocSh()->GetMedium()->GetBaseURL();

    if( aStr.Len() && aURL.GetProtocol() == INET_PROT_NOT_VALID )
        aURL = INetURLObject( ::URIHelper::SmartRel2Abs( INetURLObject(aBaseURL), aStr, URIHelper::GetMaybeFileHdl(), true, false ) );

    // get adjusted file name
    aStr = aURL.GetMainURL( INetURLObject::NO_DECODE );

    if( bFullDocDestination &&
        eCA == presentation::ClickAction_DOCUMENT &&
        aLbTreeDocument.Control::IsVisible() &&
        aLbTreeDocument.GetSelectionCount() > 0 )
    {
        String aTmpStr( aLbTreeDocument.GetSelectEntry() );
        if( aTmpStr.Len() )
        {
            aStr.Append( DOCUMENT_TOKEN );
            aStr.Append( aTmpStr );
        }
    }

    return( aStr );
}

//------------------------------------------------------------------------

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
    return( 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
