/*************************************************************************
 *
 *  $RCSfile: galbrws2.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:02:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define ITEMID_BRUSH 0

#include <sot/formats.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/valueset.hxx>
#include <svtools/urlbmk.hxx>
#include <svtools/stritem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/eitem.hxx>
#include <svtools/transfer.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxsids.hrc>
#include "impgrf.hxx"
#include "brshitem.hxx"
#include "helpid.hrc"
#include "gallery1.hxx"
#include "galtheme.hxx"
#include "galmisc.hxx"
#include "galdlg.hxx"
#include "galbrws2.hxx"

#include "svxdlg.hxx" //CHINA001
//CHINA001 #include "dialogs.hrc" //CHINA001

// -----------
// - Defines -
// -----------

#undef GALLERY_USE_CLIPBOARD

#define TBX_ID_ICON 1
#define TBX_ID_LIST 2

// -----------
// - statics -
// -----------

GalleryBrowserMode GalleryBrowser2::meInitMode = GALLERYBROWSERMODE_ICON;

// --------------------------
// - GalleryBackgroundPopup -
// --------------------------

class GalleryBackgroundPopup : public PopupMenu, public SfxControllerItem
{
    const GalleryTheme* mpTheme;
    ULONG               mnObjectPos;

    virtual void        Select();
    virtual void        StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );

public:

                    GalleryBackgroundPopup( const GalleryTheme* pTheme, ULONG nObjectPos );
                    ~GalleryBackgroundPopup();
};

// ------------------------------------------------------------------------

GalleryBackgroundPopup::GalleryBackgroundPopup( const GalleryTheme* pTheme, ULONG nObjectPos ) :
            SfxControllerItem   ( SID_GALLERY_BG_BRUSH, SfxViewFrame::Current()->GetBindings() ),
            mpTheme             ( pTheme ),
            mnObjectPos         ( nObjectPos )
{
    SfxViewFrame::Current()->GetBindings().Update( SID_GALLERY_BG_BRUSH );
    RemoveDisabledEntries();
}

// ------------------------------------------------------------------------

GalleryBackgroundPopup::~GalleryBackgroundPopup()
{
}

// ------------------------------------------------------------------------

void GalleryBackgroundPopup::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pItem )
{
    if ( ( nSID == SID_GALLERY_BG_BRUSH ) && pItem && ( eState != SFX_ITEM_DISABLED ) )
    {
        SfxStringListItem*  pStrLstItem;
        SfxStringItem*      pStrItem;

        PopupMenu::Clear();

        if( ( pStrLstItem = PTR_CAST( SfxStringListItem, pItem ) ) != NULL )
        {
            List* pList = pStrLstItem->GetList();

            if( pList )
                for ( ULONG i = 0, nCount = pList->Count(); i < nCount; i++ )
                    InsertItem( (USHORT) i + 1, *(String*) pList->GetObject( i ) );
        }
        else if( ( pStrItem = PTR_CAST( SfxStringItem, pItem ) ) != NULL )
            InsertItem( 1, pStrItem->GetValue() );
        else
        {
            DBG_ERROR( "SgaBGPopup::StateChanged(...): Wrong item type!" );
        }
    }
}

// ------------------------------------------------------------------------

void GalleryBackgroundPopup::Select()
{
    Menu::Select();

    const INetURLObject aURL( mpTheme->GetObjectURL( mnObjectPos ) );
    const SvxBrushItem  aBrushItem( aURL.GetMainURL( INetURLObject::NO_DECODE ), String(), GPOS_TILED, SID_GALLERY_BG_BRUSH );
    const SfxUInt16Item aPosItem( SID_GALLERY_BG_POS, GetCurItemId() - 1 );
    const SfxStringItem aPathItem( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::NO_DECODE ) );

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute( SID_GALLERY_BG_BRUSH,
                              SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                              &aBrushItem, &aPosItem, &aPathItem, 0L );
}

// ---------------------
// - GalleryThemePopup -
// ---------------------

class GalleryThemePopup : public PopupMenu, public SfxControllerItem
{
    GalleryBackgroundPopup  maBackgroundPopup;
    const GalleryTheme*     mpTheme;
    ULONG                   mnObjectPos;
    BOOL                    mbPreview;

    virtual void            StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );

public:

                            GalleryThemePopup( const GalleryTheme* pTheme, ULONG nObjectPos, BOOL bPreview );
                            ~GalleryThemePopup();
};

// ------------------------------------------------------------------------

GalleryThemePopup::GalleryThemePopup( const GalleryTheme* pTheme, ULONG nObjectPos, BOOL bPreview ) :
    PopupMenu           ( GAL_RESID( RID_SVXMN_GALLERY2 ) ),
    SfxControllerItem   ( SID_GALLERY_ENABLE_ADDCOPY, SfxViewFrame::Current()->GetBindings() ),
    maBackgroundPopup   ( pTheme, nObjectPos ),
    mpTheme             ( pTheme ),
    mnObjectPos         ( nObjectPos ),
    mbPreview           ( bPreview )
{
    const SgaObjKind    eObjKind = mpTheme->GetObjectKind( mnObjectPos );
    PopupMenu*          pAddMenu = GetPopupMenu( MN_ADDMENU );
    SfxBindings&        rBindings = SfxViewFrame::Current()->GetBindings();
    INetURLObject       aURL;

    const_cast< GalleryTheme* >( mpTheme )->GetURL( mnObjectPos, aURL );
    const BOOL bValidURL = ( aURL.GetProtocol() != INET_PROT_NOT_VALID );

    pAddMenu->EnableItem( MN_ADD, bValidURL && SGA_OBJ_SOUND != eObjKind );
    pAddMenu->EnableItem( MN_ADD_LINK, bValidURL && SGA_OBJ_SVDRAW != eObjKind );

    EnableItem( MN_ADDMENU, pAddMenu->IsItemEnabled( MN_ADD ) || pAddMenu->IsItemEnabled( MN_ADD_LINK ) );
    EnableItem( MN_PREVIEW, bValidURL );

    CheckItem( MN_PREVIEW, mbPreview );

    if( mpTheme->IsReadOnly() || !mpTheme->GetObjectCount() )
    {
        EnableItem( MN_DELETE, FALSE );
        EnableItem( MN_TITLE, FALSE );

        if( mpTheme->IsReadOnly() )
            EnableItem( MN_PASTECLIPBOARD, FALSE );

        if( !mpTheme->GetObjectCount() )
            EnableItem( MN_COPYCLIPBOARD, FALSE );
    }
    else
    {
        EnableItem( MN_DELETE, !bPreview );
        EnableItem( MN_TITLE, TRUE );
        EnableItem( MN_COPYCLIPBOARD, TRUE );
        EnableItem( MN_PASTECLIPBOARD, TRUE );
    }

#ifdef GALLERY_USE_CLIPBOARD
    if( IsItemEnabled( MN_PASTECLIPBOARD ) )
    {
        TransferableDataHelper  aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( this ) );
        BOOL                    bEnable = FALSE;

        if( aDataHelper.GetFormatCount() )
        {
            if( aDataHelper.HasFormat( SOT_FORMATSTR_ID_DRAWING ) ||
                aDataHelper.HasFormat( SOT_FORMAT_FILE_LIST ) ||
                aDataHelper.HasFormat( FORMAT_FILE ) ||
                aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVXB ) ||
                aDataHelper.HasFormat( FORMAT_GDIMETAFILE ) ||
                aDataHelper.HasFormat( FORMAT_BITMAP ) )
            {
                bEnable = TRUE;
            }
        }

        if( !bEnable )
            EnableItem( MN_PASTECLIPBOARD, FALSE );
    }
#else
    EnableItem( MN_COPYCLIPBOARD, FALSE );
    EnableItem( MN_PASTECLIPBOARD, FALSE );
#endif

    if( !maBackgroundPopup.GetItemCount() || ( eObjKind == SGA_OBJ_SVDRAW ) || ( eObjKind == SGA_OBJ_SOUND ) )
        pAddMenu->EnableItem( MN_BACKGROUND, FALSE );
    else
    {
        pAddMenu->EnableItem( MN_BACKGROUND, TRUE );
        pAddMenu->SetPopupMenu( MN_BACKGROUND, &maBackgroundPopup );
    }

    rBindings.Update( SID_GALLERY_ENABLE_ADDCOPY );
    RemoveDisabledEntries();
}

// ------------------------------------------------------------------------

GalleryThemePopup::~GalleryThemePopup()
{
}

// ------------------------------------------------------------------------

void GalleryThemePopup::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pItem )
{
    if( ( nSID == SID_GALLERY_ENABLE_ADDCOPY ) && pItem && ( eState != SFX_ITEM_DISABLED ) )
    {
        SfxBoolItem*        pBoolItem = PTR_CAST( SfxBoolItem, pItem );
        const SgaObjKind    eObjKind = mpTheme->GetObjectKind( mnObjectPos );

        DBG_ASSERT( pBoolItem || pBoolItem == 0, "SfxBoolItem erwartet!");

        GetPopupMenu( MN_ADDMENU )->EnableItem( MN_ADD, pBoolItem->GetValue() && ( eObjKind != SGA_OBJ_SOUND ) );
    }
}

// ------------------
// - GalleryToolBox -
// ------------------

GalleryToolBox::GalleryToolBox( GalleryBrowser2* pParent ) :
    ToolBox( pParent, WB_TABSTOP )
{
}

// ------------------------------------------------------------------------

GalleryToolBox::~GalleryToolBox()
{
}

// ------------------------------------------------------------------------

void GalleryToolBox::KeyInput( const KeyEvent& rKEvt )
{
    if( !static_cast< GalleryBrowser2* >( GetParent() )->KeyInput( rKEvt, this ) )
        ToolBox::KeyInput( rKEvt );
}

// -------------------
// - GalleryBrowser2 -
// -------------------

GalleryBrowser2::GalleryBrowser2( GalleryBrowser* pParent, const ResId& rResId, Gallery* pGallery ) :
    Control             ( pParent, rResId ),
    mpGallery           ( pGallery ),
    mpCurTheme          ( NULL ),
    mpIconView          ( new GalleryIconView( this, NULL ) ),
    mpListView          ( new GalleryListView( this, NULL ) ),
    mpPreview           ( new GalleryPreview( this, NULL ) ),
    maViewBox           ( this ),
    maSeparator         ( this, WB_VERT ),
    maInfoBar           ( this, WB_LEFT | WB_VCENTER ),
    mnCurActionPos      ( 0xffffffff ),
    meMode              ( GALLERYBROWSERMODE_NONE ),
    meLastMode          ( GALLERYBROWSERMODE_NONE ),
    mbCurActionIsLinkage( FALSE )
{
    Image       aDummyImage;
    const Link  aSelectHdl( LINK( this, GalleryBrowser2, SelectObjectHdl ) );
    Font        aInfoFont( maInfoBar.GetControlFont() );

    maViewBox.InsertItem( TBX_ID_ICON, aDummyImage );
    maViewBox.SetItemBits( TBX_ID_ICON, TIB_RADIOCHECK | TIB_AUTOCHECK );
    maViewBox.SetHelpId( TBX_ID_ICON, HID_GALLERY_ICONVIEW );
    maViewBox.SetQuickHelpText( TBX_ID_ICON, String( GAL_RESID( RID_SVXSTR_GALLERY_ICONVIEW ) ) );

    maViewBox.InsertItem( TBX_ID_LIST, aDummyImage );
    maViewBox.SetItemBits( TBX_ID_LIST, TIB_RADIOCHECK | TIB_AUTOCHECK );
    maViewBox.SetHelpId( TBX_ID_LIST, HID_GALLERY_LISTVIEW );
    maViewBox.SetQuickHelpText( TBX_ID_LIST, String( GAL_RESID( RID_SVXSTR_GALLERY_LISTVIEW ) ) );

    maViewBox.SetBorder( 0, 1 );
    maViewBox.SetOutStyle( TOOLBOX_STYLE_FLAT );
    maViewBox.SetPosSizePixel( Point(), maViewBox.CalcWindowSizePixel() );
    maViewBox.SetSelectHdl( LINK( this, GalleryBrowser2, SelectTbxHdl ) );
    maViewBox.Show();

    maInfoBar.Show();
    maSeparator.Show();

    mpIconView->SetSelectHdl( aSelectHdl );
    mpListView->SetSelectHdl( aSelectHdl );

    InitSettings();

    SetMode( ( GALLERYBROWSERMODE_PREVIEW != GalleryBrowser2::meInitMode ) ? GalleryBrowser2::meInitMode : GALLERYBROWSERMODE_ICON );
}

// -----------------------------------------------------------------------------

GalleryBrowser2::~GalleryBrowser2()
{
    delete mpPreview;
    delete mpListView;
    delete mpIconView;

    if( mpCurTheme )
        mpGallery->ReleaseTheme( mpCurTheme, *this );
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::InitSettings()
{
    BOOL  bHC = GALLERY_DLG_COLOR.IsDark();
    Image aIconImage( GAL_RESID( bHC? RID_SVXIMG_GALLERY_VIEW_ICON_HC : RID_SVXIMG_GALLERY_VIEW_ICON ) );
    Image aListImage( GAL_RESID( bHC? RID_SVXIMG_GALLERY_VIEW_LIST_HC : RID_SVXIMG_GALLERY_VIEW_LIST ) );
    Font  aInfoFont( maInfoBar.GetControlFont() );

    maViewBox.SetItemImage( TBX_ID_ICON, aIconImage );
    maViewBox.SetItemImage( TBX_ID_LIST, aListImage );

    aInfoFont.SetWeight( WEIGHT_BOLD );
    aInfoFont.SetColor( GALLERY_FG_COLOR );
    maInfoBar.SetControlFont( aInfoFont );

    maInfoBar.SetBackground( Wallpaper( GALLERY_DLG_COLOR ) );
    maInfoBar.SetControlBackground( GALLERY_DLG_COLOR );

    maSeparator.SetBackground( Wallpaper( GALLERY_BG_COLOR ) );
    maSeparator.SetControlBackground( GALLERY_BG_COLOR );
    maSeparator.SetControlForeground( GALLERY_FG_COLOR );
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings();
    else
        Control::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::Resize()
{
    Control::Resize();

    mpIconView->Hide();
    mpListView->Hide();
    mpPreview->Hide();

    const Size  aOutSize( GetOutputSizePixel() );
    const Size  aBoxSize( maViewBox.GetOutputSizePixel() );
    const long  nOffset = 2, nSepWidth = 2;
    const long  nInfoBarX = aBoxSize.Width() + ( nOffset * 3 ) + nSepWidth;
    const Point aPt( 0, aBoxSize.Height() + 3 );
    const Size  aSz( aOutSize.Width(), aOutSize.Height() - aPt.Y() );

    maSeparator.SetPosSizePixel( Point( aBoxSize.Width() + nOffset, 0 ), Size( nSepWidth, aBoxSize.Height() ) );
    maInfoBar.SetPosSizePixel( Point( nInfoBarX, 0 ), Size( aOutSize.Width() - nInfoBarX, aBoxSize.Height() ) );

    mpIconView->SetPosSizePixel( aPt, aSz );
    mpListView->SetPosSizePixel( aPt, aSz );
    mpPreview->SetPosSizePixel( aPt, aSz );

    switch( GetMode() )
    {
        case( GALLERYBROWSERMODE_ICON ): mpIconView->Show(); break;
        case( GALLERYBROWSERMODE_LIST ): mpListView->Show(); break;
        case( GALLERYBROWSERMODE_PREVIEW ): mpPreview->Show(); break;

        default:
        break;
    }
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const GalleryHint& rGalleryHint = (const GalleryHint&) rHint;

    switch( rGalleryHint.GetType() )
    {
        case( GALLERY_HINT_THEME_UPDATEVIEW ):
        {
            if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
                SetMode( meLastMode );

            ImplUpdateViews( (USHORT) rGalleryHint.GetData1() + 1 );
        }
        break;

        default:
        break;
    }
}

// -----------------------------------------------------------------------------

sal_Int8 GalleryBrowser2::AcceptDrop( DropTargetHelper& rTarget, const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( mpCurTheme && !mpCurTheme->IsReadOnly() && !mpCurTheme ->IsImported() )
    {
        if( !mpCurTheme->IsDragging() )
        {
            if( rTarget.IsDropFormatSupported( SOT_FORMATSTR_ID_DRAWING ) ||
                rTarget.IsDropFormatSupported( SOT_FORMAT_FILE_LIST ) ||
                rTarget.IsDropFormatSupported( FORMAT_FILE ) ||
                rTarget.IsDropFormatSupported( SOT_FORMATSTR_ID_SVXB ) ||
                rTarget.IsDropFormatSupported( FORMAT_GDIMETAFILE ) ||
                rTarget.IsDropFormatSupported( FORMAT_BITMAP ) )
            {
                nRet = DND_ACTION_COPY;
            }
        }
        else
            nRet = DND_ACTION_COPY;
    }

    return nRet;
}

// -----------------------------------------------------------------------------

sal_Int8 GalleryBrowser2::ExecuteDrop( DropTargetHelper& rTarget, const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( mpCurTheme )
    {
        Point       aSelPos;
        const ULONG nItemId = ImplGetSelectedItemId( &rEvt.maPosPixel, aSelPos );
        const ULONG nInsertPos = ( nItemId ? ( nItemId - 1 ) : LIST_APPEND );

        if( mpCurTheme->IsDragging() )
            mpCurTheme->ChangeObjectPos( mpCurTheme->GetDragPos(), nInsertPos );
        else
            nRet = mpCurTheme->InsertTransferable( rEvt.maDropEvent.Transferable, nInsertPos );
    }

    return nRet;
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::StartDrag( Window* pWindow, const Point* pDragPoint )
{
    if( mpCurTheme )
    {
        Point       aSelPos;
        const ULONG nItemId = ImplGetSelectedItemId( pDragPoint, aSelPos );

        if( nItemId )
            mpCurTheme->StartDrag( this, nItemId - 1 );
    }
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::TogglePreview( Window* pWindow, const Point* pPreviewPoint )
{
    SetMode( ( GALLERYBROWSERMODE_PREVIEW != GetMode() ) ? GALLERYBROWSERMODE_PREVIEW : meLastMode );
    GetViewWindow()->GrabFocus();
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::ShowContextMenu( Window* pWindow, const Point* pContextPoint )
{
    Point       aSelPos;
    const ULONG nItemId = ImplGetSelectedItemId( pContextPoint, aSelPos );

    if( mpCurTheme && nItemId && ( nItemId <= mpCurTheme->GetObjectCount() ) )
    {
        ImplSelectItemId( nItemId );

        SfxBindings& rBindings = SfxViewFrame::Current()->GetBindings();
        rBindings.ENTERREGISTRATIONS();
        GalleryThemePopup aMenu( mpCurTheme, nItemId - 1, GALLERYBROWSERMODE_PREVIEW == GetMode() );
        rBindings.LEAVEREGISTRATIONS();
        aMenu.SetSelectHdl( LINK( this, GalleryBrowser2, MenuSelectHdl ) );
        aMenu.Execute( this, aSelPos  );
    }
}

// -----------------------------------------------------------------------------

BOOL GalleryBrowser2::KeyInput( const KeyEvent& rKEvt, Window* pWindow )
{
    Point       aSelPos;
    const ULONG nItemId = ImplGetSelectedItemId( NULL, aSelPos );
    BOOL        bRet = static_cast< GalleryBrowser* >( GetParent() )->KeyInput( rKEvt, pWindow );

    if( !bRet && !maViewBox.HasFocus() && nItemId && mpCurTheme )
    {
        USHORT              nExecuteId = 0;
        const SgaObjKind    eObjKind = mpCurTheme->GetObjectKind( nItemId - 1 );
        INetURLObject       aURL;

        const_cast< GalleryTheme* >( mpCurTheme )->GetURL( nItemId - 1, aURL );

        const BOOL  bValidURL = ( aURL.GetProtocol() != INET_PROT_NOT_VALID );
        BOOL        bPreview = bValidURL;
        BOOL        bAdd = bValidURL;
        BOOL        bAddLink = ( bValidURL && SGA_OBJ_SVDRAW != eObjKind );
        BOOL        bDelete = FALSE;
        BOOL        bTitle = FALSE;

        if( !mpCurTheme->IsReadOnly() && mpCurTheme->GetObjectCount() )
        {
            bDelete = ( GALLERYBROWSERMODE_PREVIEW != GetMode() );
            bTitle = TRUE;
        }

        switch( rKEvt.GetKeyCode().GetCode() )
        {
            case( KEY_SPACE ):
            case( KEY_RETURN ):
            case( KEY_P ):
            {
                if( bPreview )
                {
                    TogglePreview( pWindow );
                    bRet = TRUE;
                }
            }
            break;

            case( KEY_INSERT ):
            case( KEY_I ):
            {
                if( bAddLink && rKEvt.GetKeyCode().IsShift() && rKEvt.GetKeyCode().IsMod1() )
                    nExecuteId = MN_ADD_LINK;
                else if( bAdd )
                    nExecuteId = MN_ADD;
            }
            break;

            case( KEY_DELETE ):
            case( KEY_D ):
            {
                if( bDelete )
                    nExecuteId = MN_DELETE;
            }
            break;

            case( KEY_T ):
            {
                if( bTitle )
                    nExecuteId = MN_TITLE;
            }
            break;

            default:
            break;
        }

        if( nExecuteId )
        {
            ImplExecute( nExecuteId );
            bRet = TRUE;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::SelectTheme( const String& rThemeName )
{
    delete mpIconView, mpIconView = NULL;
    delete mpListView, mpListView = NULL;
    delete mpPreview, mpPreview = NULL;

    if( mpCurTheme )
        mpGallery->ReleaseTheme( mpCurTheme, *this );

    mpCurTheme = mpGallery->AcquireTheme( rThemeName, *this );

    mpIconView = new GalleryIconView( this, mpCurTheme );
    mpListView = new GalleryListView( this, mpCurTheme );
    mpPreview = new GalleryPreview( this, mpCurTheme );

    const Link aSelectHdl( LINK( this, GalleryBrowser2, SelectObjectHdl ) );

    mpIconView->SetSelectHdl( aSelectHdl );
    mpListView->SetSelectHdl( aSelectHdl );

    if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
        meMode = meLastMode;

    Resize();
    ImplUpdateViews( 1 );

    maViewBox.EnableItem( TBX_ID_ICON, TRUE );
    maViewBox.EnableItem( TBX_ID_LIST, TRUE );
    maViewBox.CheckItem( ( GALLERYBROWSERMODE_ICON == GetMode() ) ? TBX_ID_ICON : TBX_ID_LIST, TRUE );
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::SetMode( GalleryBrowserMode eMode )
{
    if( GetMode() != eMode )
    {
        meLastMode = GetMode();

        switch( eMode )
        {
            case( GALLERYBROWSERMODE_ICON ):
            {
                mpListView->Hide();

                mpPreview->Hide();
                mpPreview->SetGraphic( Graphic() );
                mpPreview->PreviewMedia( INetURLObject() );

                mpIconView->Show();

                maViewBox.EnableItem( TBX_ID_ICON, TRUE );
                maViewBox.EnableItem( TBX_ID_LIST, TRUE );

                maViewBox.CheckItem( TBX_ID_ICON, TRUE );
                maViewBox.CheckItem( TBX_ID_LIST, FALSE );
            }
            break;

            case( GALLERYBROWSERMODE_LIST ):
            {
                mpIconView->Hide();

                mpPreview->Hide();
                mpPreview->SetGraphic( Graphic() );
                mpPreview->PreviewMedia( INetURLObject() );

                mpListView->Show();

                maViewBox.EnableItem( TBX_ID_ICON, TRUE );
                maViewBox.EnableItem( TBX_ID_LIST, TRUE );

                maViewBox.CheckItem( TBX_ID_ICON, FALSE );
                maViewBox.CheckItem( TBX_ID_LIST, TRUE );
            }
            break;

            case( GALLERYBROWSERMODE_PREVIEW ):
            {
                Graphic     aGraphic;
                Point       aSelPos;
                const ULONG nItemId = ImplGetSelectedItemId( NULL, aSelPos );

                if( nItemId )
                {
                    const ULONG nPos = nItemId - 1;

                       mpIconView->Hide();
                    mpListView->Hide();

                    if( mpCurTheme )
                        mpCurTheme->GetGraphic( nPos, aGraphic );

                    mpPreview->SetGraphic( aGraphic );
                     mpPreview->Show();

                    if( mpCurTheme && mpCurTheme->GetObjectKind( nPos ) == SGA_OBJ_SOUND )
                        mpPreview->PreviewMedia( mpCurTheme->GetObjectURL( nPos ) );

                    maViewBox.EnableItem( TBX_ID_ICON, FALSE );
                    maViewBox.EnableItem( TBX_ID_LIST, FALSE );
                }
            }
            break;
        }

        GalleryBrowser2::meInitMode = meMode = eMode;
    }
}

// -----------------------------------------------------------------------------

Window* GalleryBrowser2::GetViewWindow() const
{
    Window* pRet;

    switch( GetMode() )
    {
        case( GALLERYBROWSERMODE_LIST ): pRet = mpListView; break;
        case( GALLERYBROWSERMODE_PREVIEW ): pRet = mpPreview; break;

        default:
            pRet = mpIconView;
        break;
    }

    return pRet;
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::Travel( GalleryBrowserTravel eTravel )
{
    if( mpCurTheme )
    {
        Point       aSelPos;
        const ULONG nItemId = ImplGetSelectedItemId( NULL, aSelPos );

        if( nItemId )
        {
            ULONG nNewItemId = nItemId;

            switch( eTravel )
            {
                case( GALLERYBROWSERTRAVEL_FIRST ):     nNewItemId = 1; break;
                case( GALLERYBROWSERTRAVEL_LAST ):      nNewItemId = mpCurTheme->GetObjectCount(); break;
                case( GALLERYBROWSERTRAVEL_PREVIOUS ):  nNewItemId--; break;
                case( GALLERYBROWSERTRAVEL_NEXT ):      nNewItemId++; break;
            }

            if( nNewItemId < 1 )
                nNewItemId = 1;
            else if( nNewItemId > mpCurTheme->GetObjectCount() )
                nNewItemId = mpCurTheme->GetObjectCount();

            if( nNewItemId != nItemId )
            {
                ImplSelectItemId( nNewItemId );
                ImplUpdateInfoBar();

                if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
                {
                    Graphic     aGraphic;
                    const ULONG nPos = nNewItemId - 1;

                    mpCurTheme->GetGraphic( nPos, aGraphic );
                    mpPreview->SetGraphic( aGraphic );

                    if( SGA_OBJ_SOUND == mpCurTheme->GetObjectKind( nPos ) )
                        mpPreview->PreviewMedia( mpCurTheme->GetObjectURL( nPos ) );

                    mpPreview->Invalidate();
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::ImplUpdateViews( USHORT nSelectionId )
{
    mpIconView->Hide();
    mpListView->Hide();
    mpPreview->Hide();

    mpIconView->Clear();
    mpListView->Clear();

    if( mpCurTheme )
    {
        for( ULONG i = 0, nCount = mpCurTheme->GetObjectCount(); i < nCount; )
        {
            mpListView->RowInserted( i++ );
            mpIconView->InsertItem( (USHORT) i );
        }

        ImplSelectItemId( ( ( nSelectionId > mpCurTheme->GetObjectCount() ) ? mpCurTheme->GetObjectCount() : nSelectionId ) );
    }

    switch( GetMode() )
    {
        case( GALLERYBROWSERMODE_ICON ): mpIconView->Show(); break;
        case( GALLERYBROWSERMODE_LIST ): mpListView->Show(); break;
        case( GALLERYBROWSERMODE_PREVIEW ): mpPreview->Show(); break;

        default:
        break;
    }

    ImplUpdateInfoBar();
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::ImplUpdateInfoBar()
{
    String aInfoText;

    if( mpCurTheme )
    {
        Point       aSelPos;
        const ULONG nItemId = ImplGetSelectedItemId( NULL, aSelPos );

        if( nItemId )
        {
            const ULONG nPos = nItemId - 1;

            aInfoText = mpCurTheme->GetName();

            if( nPos < mpCurTheme->GetObjectCount() )
            {
                SgaObject* pObj = mpCurTheme->AcquireObject( nPos );

                if( pObj )
                {
                    aInfoText = GetItemText( *mpCurTheme, *pObj, GALLERY_ITEM_THEMENAME | GALLERY_ITEM_TITLE | GALLERY_ITEM_PATH );
                    mpCurTheme->ReleaseObject( pObj );
                }
            }
        }
    }

    maInfoBar.SetText( aInfoText );
}

// -----------------------------------------------------------------------------

ULONG GalleryBrowser2::ImplGetSelectedItemId( const Point* pSelPos, Point& rSelPos )
{
    const Size  aOutputSizePixel( GetOutputSizePixel() );
    ULONG       nRet = 0;

    if( GALLERYBROWSERMODE_PREVIEW == GetMode() )
    {
        nRet = ( ( GALLERYBROWSERMODE_ICON == meLastMode ) ? mpIconView->GetSelectItemId() : ( mpListView->FirstSelectedRow() + 1 ) );

        if( pSelPos )
            rSelPos = GetPointerPosPixel();
        else
            rSelPos = Point( aOutputSizePixel.Width() >> 1, aOutputSizePixel.Height() >> 1 );
    }
    else if( GALLERYBROWSERMODE_ICON == GetMode() )
    {
        if( pSelPos )
        {
            nRet = mpIconView->GetItemId( *pSelPos );
            rSelPos = GetPointerPosPixel();
        }
        else
        {
            nRet = mpIconView->GetSelectItemId();
            rSelPos = mpIconView->GetItemRect( (USHORT) nRet ).Center();
        }
    }
    else
    {
        if( pSelPos )
        {
            nRet = mpListView->GetRowAtYPosPixel( pSelPos->Y() ) + 1;
            rSelPos = GetPointerPosPixel();
        }
        else
        {
            nRet = mpListView->FirstSelectedRow() + 1;
            rSelPos = mpListView->GetFieldRectPixel( (USHORT) nRet, 1 ).Center();
        }
    }

    rSelPos.X() = Max( Min( rSelPos.X(), aOutputSizePixel.Width() - 1L ), 0L );
    rSelPos.Y() = Max( Min( rSelPos.Y(), aOutputSizePixel.Height() - 1L ), 0L );

    return nRet;
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::ImplSelectItemId( ULONG nItemId )
{
    if( nItemId )
    {

        mpIconView->SelectItem( (USHORT) nItemId );
        mpListView->SelectRow( nItemId - 1 );
    }
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::ImplExecute( USHORT nId )
{
    Point       aSelPos;
    const ULONG nItemId = ImplGetSelectedItemId( NULL, aSelPos );

    if( mpCurTheme && nItemId )
    {
        mnCurActionPos = nItemId - 1;

        switch( nId )
        {
            case( MN_ADD ):
            case( MN_ADD_LINK ):
            {
                sal_uInt32 nFormat;

                mbCurActionIsLinkage = ( MN_ADD_LINK == nId );

                switch( mpCurTheme->GetObjectKind( mnCurActionPos ) )
                {
                    case( SGA_OBJ_BMP ):
                    case( SGA_OBJ_ANIM ):
                    case( SGA_OBJ_INET ):
                        nFormat = SGA_FORMAT_GRAPHIC | SGA_FORMAT_STRING;
                    break;

                    case ( SGA_OBJ_SOUND ) :
                        nFormat = SGA_FORMAT_SOUND | SGA_FORMAT_STRING;
                    break;

                    case( SGA_OBJ_SVDRAW ):
                        nFormat = SGA_FORMAT_GRAPHIC | SGA_FORMAT_SVDRAW | SGA_FORMAT_STRING;
                    break;

                    default :
                    break;
                }

                const SfxUInt32Item aItem( SID_GALLERY_FORMATS, nFormat );
                SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
                    SID_GALLERY_FORMATS, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
            }
            break;

            case( MN_PREVIEW ):
                SetMode( ( GALLERYBROWSERMODE_PREVIEW != GetMode() ) ? GALLERYBROWSERMODE_PREVIEW : meLastMode );
            break;

            case( MN_DELETE ):
            {
                if( !mpCurTheme->IsReadOnly() &&
                    QueryBox( NULL, WB_YES_NO, String( GAL_RESID( RID_SVXSTR_GALLERY_DELETEOBJ ) ) ).Execute() == RET_YES )
                {
                    mpCurTheme->RemoveObject( mnCurActionPos );
                }
            }
            break;

            case( MN_TITLE ):
            {
                SgaObject* pObj = mpCurTheme->AcquireObject( mnCurActionPos );

                if( pObj )
                {
                    const String    aOldTitle( GetItemText( *mpCurTheme, *pObj, GALLERY_ITEM_TITLE ) );
                    //CHINA001 TitleDialog      aDlg( this, aOldTitle );
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    if(pFact)
                    {
                        AbstractTitleDialog* aDlg = pFact->CreateTitleDialog( this, aOldTitle, ResId(RID_SVXDLG_GALLERY_TITLE) );
                        DBG_ASSERT(aDlg, "Dialogdiet fail!");//CHINA001
                        if( aDlg->Execute() == RET_OK )//CHINA001 if( aDlg.Execute() == RET_OK )
                        {
                            String aNewTitle( aDlg->GetTitle() );//CHINA001 String aNewTitle( aDlg.GetTitle() );

                            if( ( !aNewTitle.Len() && pObj->GetTitle().Len() ) || ( aNewTitle != aOldTitle ) )
                            {
                                if( !aNewTitle.Len() )
                                    aNewTitle = String( RTL_CONSTASCII_USTRINGPARAM( "__<empty>__" ) );

                                pObj->SetTitle( aNewTitle );
                                mpCurTheme->InsertObject( *pObj );
                            }
                        }

                        mpCurTheme->ReleaseObject( pObj );
                        delete aDlg; //add CHINA001
                    }
                }
            }
            break;

            case( MN_COPYCLIPBOARD ):
            {
                Window* pWindow;

                switch( GetMode() )
                {
                    case( GALLERYBROWSERMODE_ICON ): pWindow = (Window*) mpIconView; break;
                    case( GALLERYBROWSERMODE_LIST ): pWindow = (Window*) mpListView; break;
                    case( GALLERYBROWSERMODE_PREVIEW ): pWindow = (Window*) mpPreview; break;

                    default:
                        pWindow = NULL;
                    break;
                }

                mpCurTheme->CopyToClipboard( pWindow, mnCurActionPos );
            }
            break;

            case( MN_PASTECLIPBOARD ):
            {
                if( !mpCurTheme->IsReadOnly() )
                {
                    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( this ) );
                    mpCurTheme->InsertTransferable( aDataHelper.GetTransferable(), mnCurActionPos );
                }
            }
            break;

            default:
            break;
        }
    }
}

// -----------------------------------------------------------------------------

String GalleryBrowser2::GetItemText( const GalleryTheme& rTheme, const SgaObject& rObj, ULONG nItemTextFlags )
{
    INetURLObject   aURL;
    String          aRet;

    if( rTheme.IsImported() )
    {
        aURL = rTheme.GetParent()->GetImportURL( rTheme.GetName() );

        aURL.removeSegment();
        aURL.Append( rObj.GetURL().GetName() );
    }
    else
        aURL = rObj.GetURL();

    if( nItemTextFlags & GALLERY_ITEM_THEMENAME )
    {
        aRet += rTheme.GetName();
        aRet += String( RTL_CONSTASCII_USTRINGPARAM( " - " ) );
    }

    if( nItemTextFlags & GALLERY_ITEM_TITLE )
    {
        String aTitle( rObj.GetTitle() );

        if( !aTitle.Len() )
            aTitle = aURL.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_UNAMBIGUOUS );

        if( !aTitle.Len() )
        {
            aTitle = aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
            aTitle = aTitle.GetToken( aTitle.GetTokenCount( '/' ) - 1, '/' );
        }

        aRet += aTitle;
    }

    if( nItemTextFlags & GALLERY_ITEM_PATH )
    {
        const String aPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

        if( aPath.Len() && ( nItemTextFlags & GALLERY_ITEM_TITLE ) )
            aRet += String( RTL_CONSTASCII_USTRINGPARAM( " (" ) );

        aRet += aURL.getFSysPath( INetURLObject::FSYS_DETECT );

        if( aPath.Len() && ( nItemTextFlags & GALLERY_ITEM_TITLE ) )
            aRet += ')';
    }

    return aRet;
}

// -----------------------------------------------------------------------------

INetURLObject GalleryBrowser2::GetURL() const
{
    INetURLObject aURL;

    if( mpCurTheme && mnCurActionPos != 0xffffffff )
        aURL = mpCurTheme->GetObjectURL( mnCurActionPos );

    return aURL;
}

// -----------------------------------------------------------------------------

String GalleryBrowser2::GetFilterName() const
{
    String aFilterName;

    if( mpCurTheme && mnCurActionPos != 0xffffffff )
    {
        const SgaObjKind eObjKind = mpCurTheme->GetObjectKind( mnCurActionPos );

        if( ( SGA_OBJ_BMP == eObjKind ) || ( SGA_OBJ_ANIM == eObjKind ) )
        {
            GraphicFilter*      pFilter = GetGrfFilter();
            INetURLObject       aURL; mpCurTheme->GetURL( mnCurActionPos, aURL );
            sal_uInt16          nFilter = pFilter->GetImportFormatNumberForShortName( aURL.GetExtension() );

            if( GRFILTER_FORMAT_DONTKNOW != nFilter )
                aFilterName = pFilter->GetImportFormatName( nFilter );
        }
    }

    return aFilterName;
}

// -----------------------------------------------------------------------------

Graphic GalleryBrowser2::GetGraphic() const
{
    Graphic aGraphic;

    if( mpCurTheme && mnCurActionPos != 0xffffffff )
        mpCurTheme->GetGraphic( mnCurActionPos, aGraphic );

    return aGraphic;
}

// -----------------------------------------------------------------------------

BOOL GalleryBrowser2::GetVCDrawModel( FmFormModel& rModel ) const
{
    BOOL bRet = FALSE;

    if( mpCurTheme && mnCurActionPos != 0xffffffff )
        bRet = mpCurTheme->GetModel( mnCurActionPos, rModel );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryBrowser2::IsLinkage() const
{
    return mbCurActionIsLinkage;
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryBrowser2, MenuSelectHdl, Menu*, pMenu )
{
    if( pMenu )
        ImplExecute( pMenu->GetCurItemId() );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryBrowser2, SelectObjectHdl, void*, p )
{
    ImplUpdateInfoBar();
    return 0L;
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryBrowser2, SelectTbxHdl, ToolBox*, pBox )
{
    if( pBox->GetCurItemId() == TBX_ID_ICON )
        SetMode( GALLERYBROWSERMODE_ICON );
    else if( pBox->GetCurItemId() == TBX_ID_LIST )
        SetMode( GALLERYBROWSERMODE_LIST );

    return 0L;
}
