/*************************************************************************
 *
 *  $RCSfile: galbrws2.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ka $ $Date: 2000-10-25 14:46:39 $
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
#include <vcl/drag.hxx>
#include <svtools/valueset.hxx>
#include <svtools/urlbmk.hxx>
#include <svtools/stritem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/eitem.hxx>
#include <sfx2/exchobj.hxx>
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

// -------------------
// - GalleryValueSet -
// -------------------

class GalleryValueSet : public ValueSet
{
private:

    GalleryTheme*       mpTheme;

    virtual void        UserDraw( const UserDrawEvent& rUDEvt );
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual BOOL        QueryDrop( DropEvent& rEvt );
    virtual BOOL        Drop( const DropEvent& rEvt );

public:

                        GalleryValueSet( GalleryBrowser2* pParent, GalleryTheme* pTheme, WinBits nWinStyle );
                        ~GalleryValueSet();
};

// ------------------------------------------------------------------------

GalleryValueSet::GalleryValueSet( GalleryBrowser2* pParent, GalleryTheme* pTheme, WinBits nWinStyle ) :
        ValueSet( pParent, nWinStyle ),
        mpTheme ( pTheme )
{
    EnableDrop( TRUE );
}

// ------------------------------------------------------------------------

GalleryValueSet::~GalleryValueSet()
{
}

// ------------------------------------------------------------------------

void GalleryValueSet::UserDraw( const UserDrawEvent& rUDEvt )
{
    const USHORT nId = rUDEvt.GetItemId();

    if( nId && mpTheme )
    {
        SgaObject* pObj = mpTheme->AcquireObject( nId - 1 );

        if( pObj )
        {
            const Rectangle&    rRect = rUDEvt.GetRect();
            OutputDevice*       pDev = rUDEvt.GetDevice();
            Graphic             aGraphic;

            if( pObj->IsThumbBitmap() )
            {
                Bitmap aBmp( pObj->GetThumbBmp() );

                if( pObj->GetObjKind() == SGA_OBJ_SOUND )
                    aBmp.Replace( COL_LIGHTMAGENTA, COL_WHITE );

                aGraphic = aBmp;
            }
            else
                aGraphic = pObj->GetThumbMtf();

            Size aSize( pDev->LogicToPixel( aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode() ) );

            if ( aSize.Width() && aSize.Height() )
            {
                if( ( aSize.Width() > rRect.GetWidth() ) || ( aSize.Height() > rRect.GetHeight() ) )
                {
                    Point           aNewPos;
                    const double    fBmpWH  = (double) aSize.Width() / aSize.Height();
                    const double    fThmpWH = (double) rRect.GetWidth() / rRect.GetHeight();

                    // Bitmap an Thumbgroesse anpassen
                    if ( fBmpWH < fThmpWH )
                    {
                        aSize.Width() = (long) ( rRect.GetHeight() * fBmpWH );
                        aSize.Height()= rRect.GetHeight();
                    }
                    else
                    {
                        aSize.Width() = rRect.GetWidth();
                        aSize.Height()= (long) ( rRect.GetWidth() / fBmpWH );
                    }
                }

                const Point aPos( ( ( rRect.GetWidth() - aSize.Width() ) >> 1 ) + rRect.Left(),
                                  ( ( rRect.GetHeight() - aSize.Height() ) >> 1 ) + rRect.Top() );

                aGraphic.Draw( pDev, aPos, aSize );
            }

            String aItemText;

            if( mpTheme->IsImported() )
            {
                INetURLObject aPathTmp( mpTheme->GetParent()->GetImportPath( mpTheme->GetName() ), INET_PROT_FILE );

                aPathTmp.removeSegment();
                aPathTmp.Append( INetURLObject( mpTheme->GetObjectPath( nId - 1 ), INET_PROT_FILE ).GetName() );
                aItemText = aPathTmp.PathToFileName();
            }
            else
                aItemText = mpTheme->GetObjectPath( nId - 1 );

            if( pObj->GetTitle().Len() )
            {
                String aTitleItemText( pObj->GetTitle() );

                if( pObj->GetObjKind() != SGA_OBJ_SVDRAW )
                {
                    aTitleItemText += String( RTL_CONSTASCII_USTRINGPARAM( " (" ) );
                    aTitleItemText += aItemText;
                    aTitleItemText += ')';
                }

                aItemText = aTitleItemText;
            }

            SetItemText( nId, aItemText );
            mpTheme->ReleaseObject( pObj );
        }
    }
}

// ------------------------------------------------------------------------

void GalleryValueSet::MouseButtonDown( const MouseEvent& rMEvt )
{
    ValueSet::MouseButtonDown( rMEvt );
    GetParent()->MouseButtonDown( rMEvt );
}

// ------------------------------------------------------------------------

void GalleryValueSet::Command( const CommandEvent& rCEvt )
{
    Region aRegion;

    ValueSet::Command( rCEvt );
    StartDrag( rCEvt, aRegion );
    GetParent()->Command( rCEvt );
}

// ------------------------------------------------------------------------

BOOL GalleryValueSet::QueryDrop( DropEvent& rDEvt )
{
    return( GetParent()->QueryDrop( rDEvt ) );
}

// ------------------------------------------------------------------------

BOOL GalleryValueSet::Drop( const DropEvent& rDEvt )
{
    return( GetParent()->Drop( rDEvt ) );
}

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

    const String aFilePath( mpTheme->GetObjectPath( mnObjectPos ) );
    const SvxBrushItem  aBrushItem( aFilePath, String(), GPOS_TILED, SID_GALLERY_BG_BRUSH );
    const SfxUInt16Item aPosItem( SID_GALLERY_BG_POS, GetCurItemId() - 1 );
    const SfxStringItem aPathItem( SID_FILE_NAME, aFilePath );

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

    pAddMenu->EnableItem( MN_ADD, SGA_OBJ_SOUND != eObjKind );
    pAddMenu->EnableItem( MN_ADD_LINK, SGA_OBJ_SVDRAW != eObjKind && SGA_OBJ_SOUND != eObjKind );
    EnableItem( MN_ADDMENU, pAddMenu->IsItemEnabled( MN_ADD ) || pAddMenu->IsItemEnabled( MN_ADD_LINK ) );

    CheckItem( MN_PREVIEW, mbPreview );

    if( mpTheme->IsReadOnly() || !mpTheme->GetObjectCount() )
    {
        EnableItem( MN_DELETE, FALSE );
        EnableItem( MN_TITLE, FALSE );
    }
    else
    {
        EnableItem( MN_DELETE, !bPreview );
        EnableItem( MN_TITLE, TRUE );
    }

    if( !maBackgroundPopup.GetItemCount() || ( eObjKind == SGA_OBJ_SVDRAW ) )
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
        SfxBoolItem* pBoolItem = PTR_CAST( SfxBoolItem, pItem );
        DBG_ASSERT( pBoolItem || pBoolItem == 0, "SfxBoolItem erwartet!");
        GetPopupMenu( MN_ADDMENU )->EnableItem( MN_ADD, pBoolItem->GetValue() );
    }
}

// -------------------
// - GalleryBrowser2 -
// -------------------

GalleryBrowser2::GalleryBrowser2( GalleryBrowser* pParent, const ResId& rResId, Gallery* pGallery ) :
    Control             ( pParent, rResId ),
    mpGallery           ( pGallery ),
    mpCurTheme          ( NULL ),
    mpValueSet          ( NULL ),
    mpPreview           ( NULL ),
    mnCurActionPos      ( 0xffffffff ),
    mbIsPreview         ( FALSE ),
    mbCurActionIsLinkage( FALSE )
{
}

// -----------------------------------------------------------------------------

GalleryBrowser2::~GalleryBrowser2()
{
    delete mpPreview;
    delete mpValueSet;

    if( mpCurTheme )
        mpGallery->ReleaseTheme( mpCurTheme, *this );
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::Resize()
{
    Control::Resize();

    const Size      aOutSize( GetOutputSizePixel() );
    USHORT          nSelLine = 0;
    USHORT          nSelectId = 1;
    USHORT          nColCount = mpValueSet->GetColCount();

    // Valueset und Preview-Fenster in der Groesse anpassen
    if( mbIsPreview )
        mpPreview->Hide();
    else
        mpValueSet->Hide();

    mpValueSet->SetSizePixel( aOutSize );
    mpPreview->SetSizePixel( aOutSize );

    if( mbIsPreview )
        mpPreview->Show();
    else
        mpValueSet->Show();
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const GalleryHint& rGalleryHint = (const GalleryHint&) rHint;

    switch( rGalleryHint.GetType() )
    {
        case( GALLERY_HINT_THEME_UPDATEVIEW ):
            ImplUpdateValueSet( rGalleryHint.GetData1() + 1 );
        break;

        default:
        break;
    }
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.GetClicks() > 1 )
        ShowPreview( !mbIsPreview );
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::Command( const CommandEvent& rCEvt )
{
    const USHORT nId = mbIsPreview ? mpValueSet->GetSelectItemId() : mpValueSet->GetItemId( rCEvt.GetMousePosPixel() );

    if( rCEvt.GetCommand() == COMMAND_STARTDRAG )
    {
        SgaDataObjectRef    xDataObject( new SgaDataObject( mpCurTheme, nId - 1 ) );
        Region              aRegion( mpValueSet->GetItemRect( nId ) );
        const DropAction    eAction = xDataObject->ExecuteDrag( NULL,
                                            POINTER_MOVEDATA, POINTER_COPYDATA, POINTER_LINKDATA,
                                            DRAG_COPYABLE | DRAG_LINKABLE /*| DRAG_MOVEABLE*/,
                                            &aRegion );
    }
    else if( ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU ) && rCEvt.IsMouseEvent() && mpCurTheme )
    {
        if( !mbIsPreview )
            mpValueSet->SelectItem( nId );

        SfxBindings& rBindings = SfxViewFrame::Current()->GetBindings();
        rBindings.ENTERREGISTRATIONS();
        GalleryThemePopup aMenu( mpCurTheme, nId - 1, mbIsPreview );
        rBindings.LEAVEREGISTRATIONS();
        aMenu.SetSelectHdl( LINK( this, GalleryBrowser2, MenuSelectHdl ) );
        aMenu.Execute( this, rCEvt.GetMousePosPixel() );
    }
}

// -----------------------------------------------------------------------------

BOOL GalleryBrowser2::QueryDrop( DropEvent& rDEvt )
{
    BOOL bRet = FALSE;

    if( mpCurTheme && !mpCurTheme->IsReadOnly() && !mpCurTheme ->IsImported() )
    {
        bRet = mpCurTheme->IsDragging();

        if( !bRet )
        {
            USHORT i = 0, nCount = DragServer::GetItemCount();

            while( !bRet && ( i < nCount ) )
            {
                bRet = DragServer::HasFormat( i, FORMAT_BITMAP ) ||
                       DragServer::HasFormat( i, FORMAT_GDIMETAFILE ) ||
                       DragServer::HasFormat( i, FORMAT_FILE ) ||
                       DragServer::HasFormat( i, SOT_FORMATSTR_ID_SVXB ) ||
                       DragServer::HasFormat( i, SOT_FORMATSTR_ID_DRAWING ) ||
                       INetBookmark::DragServerHasFormat( i );

                i++;
            }

            if( rDEvt.IsDefaultAction() )
                rDEvt.SetAction( DROP_COPY );
        }
        else if( rDEvt.IsDefaultAction() )
            rDEvt = DropEvent( rDEvt.GetPosPixel(), rDEvt.GetData(), DROP_MOVE, DROP_MOVE, rDEvt.GetWindowType(), rDEvt.IsDefaultAction() );
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryBrowser2::Drop( const DropEvent& rDEvt )
{
    BOOL bRet = FALSE;

    if( mpCurTheme )
    {
        const USHORT    nItemId = mbIsPreview ? mpValueSet->GetSelectItemId() : mpValueSet->GetItemId( rDEvt.GetPosPixel() );
        const ULONG     nInsertPos = ( nItemId ? ( nItemId - 1 ) : LIST_APPEND );

        if( mpCurTheme->IsDragging() )
            mpCurTheme->ChangeObjectPos( mpCurTheme->GetDragPos(), nInsertPos );
        else
            mpCurTheme->InsertDataXChgData( SvDataObject::PasteDragServer( rDEvt ) );

        bRet = FALSE;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::SelectTheme( const String& rThemeName )
{
    delete mpValueSet, mpValueSet = NULL;
    delete mpPreview, mpPreview = NULL;
    mbIsPreview = FALSE;

    if( mpCurTheme )
        mpGallery->ReleaseTheme( mpCurTheme, *this );

    mpCurTheme = mpGallery->AcquireTheme( rThemeName, *this );

    mpValueSet = new GalleryValueSet( this, mpCurTheme, WB_3DLOOK | WB_BORDER |
                                                        WB_ITEMBORDER | WB_DOUBLEBORDER |
                                                        WB_NAMEFIELD | WB_VSCROLL |
                                                        WB_FLATVALUESET );
    mpValueSet->SetBackground( Wallpaper( COL_WHITE ) );
    mpValueSet->SetControlBackground( COL_WHITE );
    mpValueSet->SetColor( COL_WHITE );
    mpValueSet->SetHelpId( HID_GALLERY_WINDOW );
    mpValueSet->SetPosSizePixel( Point(), GetOutputSizePixel() );
    mpValueSet->SetSelectHdl( LINK( this, GalleryBrowser2, SelectObjectHdl ) );
    mpValueSet->SetDoubleClickHdl( LINK( this, GalleryBrowser2, DoubleClickObjectHdl ) );
    mpValueSet->SetExtraSpacing( 2 );
    mpValueSet->SetItemWidth( S_THUMB + 6 );
    mpValueSet->SetItemHeight( S_THUMB + 6 );

    mpPreview = new GalleryPreview( this, WB_BORDER ),
    mpPreview->SetHelpId( HID_GALLERY_WINDOW );
    mpPreview->Show( FALSE );

    ImplUpdateValueSet( 1 );
    Resize();
    mpValueSet->Show( TRUE );
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::ShowPreview( BOOL bShow )
{
    if( mpCurTheme && mpValueSet && mpPreview )
    {
        if( mbIsPreview && !bShow )
        {
            mpPreview->Show( FALSE );
            mpValueSet->Show( TRUE );

            mpPreview->SetGraphic( Graphic() );
            mpPreview->PreviewSound( String() );

            mbIsPreview = FALSE;
        }
        else if( !mbIsPreview && bShow )
        {
            Graphic     aGraphic;
            const ULONG nPos = mpValueSet->GetSelectItemId() - 1;

            if( mpCurTheme->GetGraphic( nPos, aGraphic ) &&
                aGraphic.GetType() != GRAPHIC_NONE )
            {
                mpPreview->SetGraphic( aGraphic );

                mpValueSet->Show( FALSE );
                mpPreview->Show( TRUE );

                if( mpCurTheme->GetObjectKind( nPos ) == SGA_OBJ_SOUND )
                    mpPreview->PreviewSound( mpCurTheme->GetObjectPath( nPos ) );

                mbIsPreview = TRUE;
            }
        }
    }
}

// -----------------------------------------------------------------------------

void GalleryBrowser2::ImplUpdateValueSet( USHORT nSelectionId )
{
    USHORT nCurItemId = mpValueSet->GetSelectItemId();

    mpValueSet->Clear();
    mpPreview->Show( FALSE);

    if( mpCurTheme )
    {
        for( ULONG i = 0, nCount = mpCurTheme->GetObjectCount(); i < nCount; )
            mpValueSet->InsertItem( ++i );
    }

    mpValueSet->SelectItem( ( ( nSelectionId > mpValueSet->GetItemCount() ) ? mpValueSet->GetItemCount() : nSelectionId ) );
}

// -----------------------------------------------------------------------------

INetURLObject GalleryBrowser2::GetURL() const
{
    INetURLObject aURL;

    if( mpCurTheme && mnCurActionPos != 0xffffffff )
        aURL.SetSmartURL( mpCurTheme->GetObjectPath( mnCurActionPos ) );

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
            const String        aExt( aURL.GetExtension() );
            Config&             rConfig = pFilter->GetConfig();
            const ByteString    aOldGroup( rConfig.GetGroup() );
            USHORT              nFilter = GRFILTER_FORMAT_DONTKNOW;

            rConfig.SetGroup( IMP_FILTERSECTION );

            for( USHORT i = 0, nKeyCount = rConfig.GetKeyCount(); i < nKeyCount; i++ )
            {
                const String aStr( rConfig.ReadKey( i ).GetToken(2, ',').GetBuffer(), RTL_TEXTENCODING_UTF8 );

                if( aStr.CompareIgnoreCaseToAscii( aExt ) == COMPARE_EQUAL )
                {
                    nFilter = i;
                    break;
                }
            }

            if( GRFILTER_FORMAT_DONTKNOW != nFilter )
                aFilterName = pFilter->GetImportFormatName( nFilter );

            rConfig.SetGroup( aOldGroup );
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
    {
        const USHORT nId = pMenu->GetCurItemId();

        mnCurActionPos = mpValueSet->GetSelectItemId() - 1;

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
                ShowPreview( !mbIsPreview );
            break;

            case( MN_DELETE ):
            {
                if( !mpCurTheme->IsReadOnly() &&
                    QueryBox( this, WB_YES_NO, String( GAL_RESID( RID_SVXSTR_GALLERY_DELETEOBJ ) ) ).Execute() == RET_YES )
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
                    TitleDialog aDlg( this, pObj->GetTitle() );

                    if( aDlg.Execute() == RET_OK )
                    {
                        const String aNewTitle( aDlg.GetTitle() );

                        if( aNewTitle != pObj->GetTitle() )
                        {
                            pObj->SetTitle( aNewTitle );
                            mpCurTheme->InsertObject( *pObj );
                        }
                    }

                    mpCurTheme->ReleaseObject( pObj );
                }
            }
            break;

            default:
            break;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryBrowser2, SelectObjectHdl, void*, p )
{
    return 0L;
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryBrowser2, DoubleClickObjectHdl, void*, p )
{
    return 0L;
}
