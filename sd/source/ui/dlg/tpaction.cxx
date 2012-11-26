/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include <svx/svxids.hrc>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#ifndef _COM_SUN_STAR_EMBED_VERBDESCR_HPP_
#include <com/sun/star/embed/VerbDescriptor.hpp>
#endif
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/embed/VerbAttributes.hpp>

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
#include <svx/svdograf.hxx>
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

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SdActionDlg::SdActionDlg (
    ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView ) :
        SfxSingleTabDialog  ( pParent, *pAttr, TP_ANIMATION_ACTION ),
        rOutAttrs           ( *pAttr )
{
    // FreeResource();
    SfxTabPage* pNewPage = SdTPAction::Create( this, rOutAttrs );
    DBG_ASSERT( pNewPage, "Seite konnte nicht erzeugt werden");

    // Ehemals in PageCreated
    ( (SdTPAction*) pNewPage )->SetView( pView );
    ( (SdTPAction*) pNewPage )->Construct();

    SetTabPage( pNewPage );

    String aStr( pNewPage->GetText() );
    if( aStr.Len() )
        SetText( aStr );
}


/*************************************************************************
|*
|*  Action-TabPage
|*
\************************************************************************/

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
        bTreeUpdated    ( false )
{
    aEdtSound.SetAccessibleName(String(SdResId(STR_PATHNAME)));
    aBtnSeek.SetAccessibleRelationMemberOf( &aFlSeparator );

    FreeResource();

    aBtnSearch.SetClickHdl( LINK( this, SdTPAction, ClickSearchHdl ) );
    aBtnSeek.SetClickHdl( LINK( this, SdTPAction, ClickSearchHdl ) );

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    aLbAction.SetSelectHdl( LINK( this, SdTPAction, ClickActionHdl ) );
    aLbTree.SetSelectHdl( LINK( this, SdTPAction, SelectTreeHdl ) );
    aEdtDocument.SetLoseFocusHdl( LINK( this, SdTPAction, CheckFileHdl ) );
    aEdtMacro.SetLoseFocusHdl( LINK( this, SdTPAction, CheckFileHdl ) );

    // Controls enablen
    aFtAction.Show();
    aLbAction.Show();

    ClickActionHdl( this );
}

// -----------------------------------------------------------------------

SdTPAction::~SdTPAction()
{
    delete pCurrentActions;
}

// -----------------------------------------------------------------------

void SdTPAction::SetView( const ::sd::View* pSdView )
{
    mpView = pSdView;

    // Holen der ColorTable und Fuellen der ListBox
    ::sd::DrawDocShell* pDocSh = static_cast<const ::sd::View*>(mpView)->GetDocSh();
    if( pDocSh && pDocSh->GetViewShell() )
    {
        mpDoc = pDocSh->GetDoc();
        SfxViewFrame* pFrame = pDocSh->GetViewShell()->GetViewFrame();
        aLbTree.SetViewFrame( pFrame );
        aLbTreeDocument.SetViewFrame( pFrame );

        SvxColorTableItem aItem( *(const SvxColorTableItem*)( pDocSh->GetItem( SID_COLOR_TABLE ) ) );
        pColTab = aItem.GetColorTable();
        DBG_ASSERT( pColTab, "Keine Farbtabelle vorhanden!" );
    }
    else
    {
        DBG_ERROR("sd::SdTPAction::SetView(), no docshell or viewshell?");
    }
}

// -----------------------------------------------------------------------

void SdTPAction::Construct()
{
    // OLE-Actionlistbox auffuellen
    SdrOle2Obj* pOleObj = dynamic_cast< SdrOle2Obj* >(mpView->getSelectedIfSingle());
    SdrGrafObj* pGrafObj = dynamic_cast< SdrGrafObj* >(mpView->getSelectedIfSingle());
    bool bOLEAction = false;

    if( pGrafObj )
    {
        bOLEAction = true;

        aVerbVector.push_back( 0 );
        aLbOLEAction.InsertEntry( MnemonicGenerator::EraseAllMnemonicChars( String( SdResId( STR_EDIT_OBJ ) ) ) );
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
                    String aTmp( aVerb.VerbName );
                    aVerbVector.push_back( aVerb.VerbID );
                    aLbOLEAction.InsertEntry( MnemonicGenerator::EraseAllMnemonicChars( aTmp ) );
                }
            }
        }
    }

    pCurrentActions = new List;
    pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_NONE, LIST_APPEND);
    pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_PREVPAGE, LIST_APPEND);
    pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_NEXTPAGE, LIST_APPEND);
    pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_FIRSTPAGE, LIST_APPEND);
    pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_LASTPAGE, LIST_APPEND);
    pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_BOOKMARK, LIST_APPEND);
    pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_DOCUMENT, LIST_APPEND);
    pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_SOUND, LIST_APPEND);
    if( bOLEAction && aLbOLEAction.GetEntryCount() )
        pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_VERB, LIST_APPEND );
    pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_PROGRAM, LIST_APPEND);
    pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_MACRO, LIST_APPEND);
    pCurrentActions->Insert((void*)(sal_uIntPtr)presentation::ClickAction_STOPPRESENTATION, LIST_APPEND);

    // Action-Listbox fuellen
    for (sal_uLong nAction = 0; nAction < pCurrentActions->Count(); nAction++)
    {
        sal_uInt16 nRId = GetClickActionSdResId((presentation::ClickAction)(sal_uLong)pCurrentActions->GetObject(nAction));
        aLbAction.InsertEntry( String( SdResId( nRId ) ) );
    }

}

// -----------------------------------------------------------------------

sal_Bool SdTPAction::FillItemSet( SfxItemSet& rAttrs )
{
    bool bModified = false;
    presentation::ClickAction eCA = presentation::ClickAction_NONE;

    if( aLbAction.GetSelectEntryCount() )
        eCA = GetActualClickAction();

    if( aLbAction.GetSavedValue() != aLbAction.GetSelectEntryPos() )
    {
        rAttrs.Put( SfxAllEnumItem( ATTR_ACTION, (sal_uInt16)eCA ) );
        bModified = true;
    }
    else
        rAttrs.InvalidateItem( ATTR_ACTION );

    String aFileName = GetEditText( true );
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
            bModified = true;
        }
        else
        {
            DBG_ERROR("sd::SdTPAction::FillItemSet(), I need a medium!");
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
                aLbTree.SelectAll( false );
        }
        break;

        case presentation::ClickAction_DOCUMENT:
        {
            if( aFileName.GetTokenCount( DOCUMENT_TOKEN ) == 2 )
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
        aLbTree.Fill( mpDoc, true, mpDoc->GetDocSh()->GetMedium()->GetName() );
        bTreeUpdated = true;
    }
}

//------------------------------------------------------------------------

void SdTPAction::OpenFileDialog()
{
    // Soundpreview nur fuer Interaktionen mit Sound
    presentation::ClickAction eCA = GetActualClickAction();
    bool bSound = ( eCA == presentation::ClickAction_SOUND );
    bool bPage = ( eCA == presentation::ClickAction_BOOKMARK );
    bool bDocument = ( eCA == presentation::ClickAction_DOCUMENT || eCA == presentation::ClickAction_PROGRAM );
    bool bMacro = ( eCA == presentation::ClickAction_MACRO );

    if( bPage )
    {
        // Es wird in der TreeLB nach dem eingegebenen Objekt gesucht
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
            ::rtl::OUString aScriptURL = SfxApplication::ChooseScript();

            if ( aScriptURL.getLength() != 0 )
            {
                SetEditText( aScriptURL );
            }

            Application::SetDefDialogParent( pOldWin );
        }
        else
        {
            sfx2::FileDialogHelper aFileDialog(WB_OPEN | WB_3DLOOK | WB_STDMODAL );

            if (bDocument && !aFile.Len())
                aFile = SvtPathOptions().GetWorkPath();

            aFileDialog.SetDisplayDirectory( aFile );

            // The following is a fix for #1008001# and a workarround for
            // #i4306#: The addition of the implicitely existing "all files"
            // filter makes the (Windows system) open file dialog follow
            // links on the desktop to directories.
            aFileDialog.AddFilter (
                String (SfxResId (STR_SFX_FILTERNAME_ALL)),
                String (RTL_CONSTASCII_USTRINGPARAM("*.*")));


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

IMPL_LINK( SdTPAction, ClickSearchHdl, void *, EMPTYARG )
{
    OpenFileDialog();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SdTPAction, ClickActionHdl, void *, EMPTYARG )
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

IMPL_LINK( SdTPAction, SelectTreeHdl, void *, EMPTYARG )
{
    aEdtBookmark.SetText( aLbTree.GetSelectEntry() );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SdTPAction, CheckFileHdl, void *, EMPTYARG )
{
    String aFile( GetEditText() );

    if( aFile != aLastFile )
    {
        // Ueberpruefen, ob es eine gueltige Draw-Datei ist
        SfxMedium aMedium( aFile,
                    STREAM_READ | STREAM_NOCREATE,
                    true );               // Download

        if( aMedium.IsStorage() )
        {
            WaitObject aWait( GetParent()->GetParent() );

            // ist es eine Draw-Datei?
            // mit READ oeffnen, sonst schreiben die Storages evtl. in die Datei!
            uno::Reference < embed::XStorage > xStorage = aMedium.GetStorage();
            DBG_ASSERT( xStorage.is(), "Kein Storage!" );

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
                    aLbTreeDocument.Fill( pBookmarkDoc, true, aFile );
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

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
        eCA = (presentation::ClickAction)(sal_uLong)pCurrentActions->GetObject((sal_uLong)nPos);
    return( eCA );
}

//------------------------------------------------------------------------

void SdTPAction::SetActualClickAction( presentation::ClickAction eCA )
{
    sal_uInt16 nPos = (sal_uInt16)pCurrentActions->GetPos((void*)(sal_uLong)eCA);
    DBG_ASSERT(nPos != 0xffff, "unbekannte Interaktion");
    aLbAction.SelectEntryPos(nPos);
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
            if( rStr.GetTokenCount( DOCUMENT_TOKEN ) == 2 )
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

String SdTPAction::GetEditText( bool bFullDocDestination )
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
                    aStr = UniString::CreateFromInt32( aVerbVector[ nPos ] );
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
        default: DBG_ERROR( "Keine StringResource fuer ClickAction vorhanden!" );
    }
    return( 0 );
}

