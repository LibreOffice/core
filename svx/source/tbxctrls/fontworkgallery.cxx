/*************************************************************************
 *
 *  $RCSfile: fontworkgallery.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 09:25:52 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HPP_
#include <com/sun/star/text/WritingMode.hpp>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SVX_FMMODEL_HXX
#include <fmmodel.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#include <dlgutil.hxx>

#include "svxids.hrc"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "gallery.hxx"
#include "svdpage.hxx"
#include "svdobj.hxx"
#include "svdview.hxx"
#include "svdoutl.hxx"
#include "eeitem.hxx"
#define ITEMID_FRAMEDIR EE_PARA_WRITINGDIR
#include "frmdiritem.hxx"
#include "toolbarmenu.hxx"

#include "fontworkgallery.hxx"
#include "fontworkgallery.hrc"

#include <algorithm>

#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _SVX_HELPID_HRC
#include "helpid.hrc"
#endif
using namespace svx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

const int nColCount = 4;
const int nLineCount = 4;

/*************************************************************************
|*  Svx3DWin - FloatingWindow
\************************************************************************/
FontWorkGalleryDialog::FontWorkGalleryDialog( SdrView* pSdrView, Window* pParent, sal_uInt16 nSID ) :
        ModalDialog( pParent, SVX_RES( RID_SVX_MDLG_FONTWORK_GALLERY ) ),
        maCtlFavorites      ( this, SVX_RES( CTL_FAVORITES ) ),
        maFLFavorites       ( this, SVX_RES( FL_FAVORITES ) ),
        maOKButton          ( this, SVX_RES( BTN_OK ) ),
        maCancelButton      ( this, SVX_RES( BTN_CANCEL ) ),
        maHelpButton        ( this, SVX_RES( BTN_HELP ) ),
        mnThemeId           ( -1 ),
        maStrClickToAddText ( SVX_RES( STR_CLICK_TO_ADD_TEXT ) ),
        mpSdrView           ( pSdrView ),
        mpModel             ( (FmFormModel*)pSdrView->GetModel() ),
        mppSdrObject        ( NULL ),
        mpDestModel         ( NULL )
{
    FreeResource();

    maCtlFavorites.SetDoubleClickHdl( LINK( this, FontWorkGalleryDialog, DoubleClickFavoriteHdl ) );
    maOKButton.SetClickHdl( LINK( this, FontWorkGalleryDialog, ClickOKHdl ) );

    maCtlFavorites.SetColCount( nColCount );
    maCtlFavorites.SetLineCount( nLineCount );
    maCtlFavorites.SetExtraSpacing( 3 );

    initfavorites( GALLERY_THEME_FONTWORK, maFavoritesHorizontal );
    fillFavorites( GALLERY_THEME_FONTWORK, maFavoritesHorizontal );
}

static void delete_bitmap( Bitmap* p ) { delete p; }

// -----------------------------------------------------------------------
FontWorkGalleryDialog::~FontWorkGalleryDialog()
{
    std::for_each( maFavoritesHorizontal.begin(), maFavoritesHorizontal.end(), delete_bitmap );
}

// -----------------------------------------------------------------------

void FontWorkGalleryDialog::initfavorites(sal_uInt16 nThemeId, std::vector< Bitmap * >& rFavorites)
{
    // Ueber die Gallery werden die Favoriten eingelesen
    ULONG nFavCount = GalleryExplorer::GetSdrObjCount( nThemeId );

    // Gallery thema locken
    GalleryExplorer::BeginLocking(nThemeId);

    sal_uInt32 nModelPos;
    FmFormModel *pModel = NULL;
    for( nModelPos = 0; nModelPos < nFavCount; nModelPos++ )
    {
        Bitmap* pThumb = new Bitmap;

        if( GalleryExplorer::GetSdrObj( nThemeId, nModelPos, pModel, pThumb ) )
        {
/*
            VirtualDevice aVDev;
            Size aRenderSize( aThumbSize.Width() * 4, aThumbSize.Height() * 4 );
            aVDev.SetOutputSizePixel( aRenderSize );

            if( GalleryExplorer::DrawCentered( &aVDev, *pModel ) )
            {
                aThumb = aVDev.GetBitmap( Point(), aVDev.GetOutputSizePixel() );

                Size aMS( 4, 4 );
                BmpFilterParam aParam( aMS );
                aThumb.Filter( BMP_FILTER_MOSAIC, &aParam );
                aThumb.Scale( aThumbSize );
            }
*/
        }

        rFavorites.push_back( pThumb );
    }

    // Gallery thema freigeben
    GalleryExplorer::EndLocking(nThemeId);
}

void FontWorkGalleryDialog::fillFavorites( sal_uInt16 nThemeId, std::vector< Bitmap * >& rFavorites )
{
    mnThemeId = nThemeId;

    Size aThumbSize( maCtlFavorites.GetSizePixel() );
    aThumbSize.Width() /= nColCount;
    aThumbSize.Height() /= nLineCount;
    aThumbSize.Width() -= 12;
    aThumbSize.Height() -= 12;

    sal_uInt16 nFavCount = rFavorites.size();

    // ValueSet Favoriten
    if( nFavCount > (nColCount * nLineCount) )
    {
        WinBits nWinBits = maCtlFavorites.GetStyle();
        nWinBits |= WB_VSCROLL;
        maCtlFavorites.SetStyle( nWinBits );
    }

    maCtlFavorites.Clear();

    sal_uInt32 nFavorite;
    for( nFavorite = 1; nFavorite <= nFavCount; nFavorite++ )
    {
        String aStr(SVX_RES(RID_SVXFLOAT3D_FAVORITE));
        aStr += sal_Unicode(' ');
        aStr += String::CreateFromInt32((sal_Int32)nFavorite);
        Image aThumbImage( *rFavorites[nFavorite-1] );
        maCtlFavorites.InsertItem( (sal_uInt16)nFavorite, aThumbImage, aStr );
    }
}

void FontWorkGalleryDialog::changeText( SdrTextObj* pObj )
{
    if( pObj )
    {
        SdrOutliner& rOutl = mpModel->GetDrawOutliner(pObj);

        rOutl.SetMinDepth(0);

        USHORT nOutlMode = rOutl.GetMode();
        USHORT nMinDepth = rOutl.GetMinDepth();
        Size aPaperSize = rOutl.GetPaperSize();
        BOOL bUpdateMode = rOutl.GetUpdateMode();
        rOutl.SetUpdateMode(FALSE);
        rOutl.SetParaAttribs( 0, rOutl.GetEmptyItemSet() );

        // #95114# Always set the object's StyleSheet at the Outliner to
        // use the current objects StyleSheet. Thus it's the same as in
        // SetText(...).
        // #95114# Moved this implementation from where SetObjText(...) was called
        // to inside this method to work even when outliner is fetched here.
        rOutl.SetStyleSheet(0, pObj->GetStyleSheet());

        rOutl.SetPaperSize( pObj->GetLogicRect().GetSize() );

        rOutl.SetText( maStrClickToAddText, rOutl.GetParagraph( 0 ) );
        pObj->SetOutlinerParaObject( rOutl.CreateParaObject() );

        rOutl.Init( nOutlMode );
        rOutl.SetParaAttribs( 0, rOutl.GetEmptyItemSet() );
        rOutl.SetUpdateMode( bUpdateMode );
        rOutl.SetMinDepth( nMinDepth );
        rOutl.SetPaperSize( aPaperSize );

        rOutl.Clear();
    }
}

void FontWorkGalleryDialog::SetSdrObjectRef( SdrObject** ppSdrObject, SdrModel* pModel )
{
    mppSdrObject = ppSdrObject;
    mpDestModel = pModel;
}

void FontWorkGalleryDialog::insertSelectedFontwork()
{
    USHORT nItemId = maCtlFavorites.GetSelectItemId();

    if( nItemId > 0 )
    {
        FmFormModel* pModel = new FmFormModel();
        pModel->GetItemPool().FreezeIdRanges();

        if( GalleryExplorer::GetSdrObj( mnThemeId, nItemId-1, pModel ) )
        {
            SdrPage* pPage = pModel->GetPage(0);
            if( pPage && pPage->GetObjCount() )
            {
                SdrObject* pNewObject = pPage->GetObj(0)->Clone();

                // center shape on current view
                OutputDevice* pOutDev = mpSdrView->GetWin(0);
                if( pOutDev )
                {
                    Rectangle aObjRect( pNewObject->GetLogicRect() );
                    Rectangle aVisArea = pOutDev->PixelToLogic(Rectangle(Point(0,0), pOutDev->GetOutputSizePixel()));
/*
                    sal_Int32 nObjHeight = aObjRect.GetHeight();
                    VirtualDevice aVirDev( 1 ); // calculating the optimal textwidth
                    Font aFont;
                    aFont.SetHeight( nObjHeight );
                    aVirDev.SetMapMode( MAP_100TH_MM );
                    aVirDev.SetFont( aFont );
                    aObjRect.SetSize( Size( aVirDev.GetTextWidth( maStrClickToAddText ), nObjHeight ) );
*/
                    Point aPagePos = aVisArea.Center();
                    aPagePos.X() -= aObjRect.GetWidth() / 2;
                    aPagePos.Y() -= aObjRect.GetHeight() / 2;
                    Rectangle aNewObjectRectangle(aPagePos, aObjRect.GetSize());
                    SdrPageView* pPV = mpSdrView->GetPageViewPvNum(0);

                    pNewObject->SetLogicRect(aNewObjectRectangle);
                    if ( mppSdrObject )
                    {
                        *mppSdrObject = pNewObject;
                        (*mppSdrObject)->SetModel( mpDestModel );
                    }
                    else if( pPV )
                    {
                            mpSdrView->InsertObject( pNewObject, *pPV );
    //                      changeText( PTR_CAST( SdrTextObj, pNewObject ) );
                    }
                }
            }
        }

        delete pModel;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( FontWorkGalleryDialog, ClickOKHdl, void*, p )
{
    insertSelectedFontwork();
    EndDialog( true );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( FontWorkGalleryDialog, DoubleClickFavoriteHdl, void*, p )
{
    insertSelectedFontwork();
    EndDialog( true );
    return( 0L );
}

// -----------------------------------------------------------------------

SFX_IMPL_TOOLBOX_CONTROL( FontWorkShapeTypeControl, SfxStringItem );
FontWorkShapeTypeControl::FontWorkShapeTypeControl( USHORT nSlotId, USHORT nId, ToolBox &rTbx )
: SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

// -----------------------------------------------------------------------

FontWorkShapeTypeControl::~FontWorkShapeTypeControl()
{
}

// -----------------------------------------------------------------------

SfxPopupWindowType FontWorkShapeTypeControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK; //( aLastAction.getLength() == 0 ? SFX_POPUPWINDOW_ONCLICK : SFX_POPUPWINDOW_ONTIMEOUT );
}

// -----------------------------------------------------------------------

SfxPopupWindow* FontWorkShapeTypeControl::CreatePopupWindow()
{
    rtl::OUString aSubTbxResName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/fontworkshapetype" ) );
    createAndPositionSubToolBar( aSubTbxResName );
    return NULL;
}

// -----------------------------------------------------------------------

void FontWorkShapeTypeControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    SfxToolBoxControl::StateChanged( nSID, eState, pState );
}

// -----------------------------------------------------------------------

void FontWorkShapeTypeControl::Select( BOOL bMod1 )
{

}

// ####################################################################

SFX_IMPL_TOOLBOX_CONTROL( FontWorkAlignmentControl, SfxBoolItem );

FontWorkAlignmentWindow::FontWorkAlignmentWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame ) :

    SfxPopupWindow( nId,
                    rFrame,
                    SVX_RES( RID_SVXFLOAT_FONTWORK_ALIGNMENT )),
    maImgAlgin1( SVX_RES( IMG_FONTWORK_ALIGN_LEFT_16 ) ),
    maImgAlgin2( SVX_RES( IMG_FONTWORK_ALIGN_CENTER_16 ) ),
    maImgAlgin3( SVX_RES( IMG_FONTWORK_ALIGN_RIGHT_16 ) ),
    maImgAlgin4( SVX_RES( IMG_FONTWORK_ALIGN_WORD_16 ) ),
    maImgAlgin5( SVX_RES( IMG_FONTWORK_ALIGN_STRETCH_16 ) ),
    maImgAlgin1h( SVX_RES( IMG_FONTWORK_ALIGN_LEFT_16_H ) ),
    maImgAlgin2h( SVX_RES( IMG_FONTWORK_ALIGN_CENTER_16_H ) ),
    maImgAlgin3h( SVX_RES( IMG_FONTWORK_ALIGN_RIGHT_16_H ) ),
    maImgAlgin4h( SVX_RES( IMG_FONTWORK_ALIGN_WORD_16_H ) ),
    maImgAlgin5h( SVX_RES( IMG_FONTWORK_ALIGN_STRETCH_16_H ) ),
    mbPopupMode( true ),
    mxFrame( rFrame )
{
    implInit();
}

FontWorkAlignmentWindow::FontWorkAlignmentWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParentWindow ) :

    SfxPopupWindow( nId,
                    rFrame,
                    SVX_RES( RID_SVXFLOAT_FONTWORK_ALIGNMENT )),
    maImgAlgin1( SVX_RES( IMG_FONTWORK_ALIGN_LEFT_16 ) ),
    maImgAlgin2( SVX_RES( IMG_FONTWORK_ALIGN_CENTER_16 ) ),
    maImgAlgin3( SVX_RES( IMG_FONTWORK_ALIGN_RIGHT_16 ) ),
    maImgAlgin4( SVX_RES( IMG_FONTWORK_ALIGN_WORD_16 ) ),
    maImgAlgin5( SVX_RES( IMG_FONTWORK_ALIGN_STRETCH_16 ) ),
    maImgAlgin1h( SVX_RES( IMG_FONTWORK_ALIGN_LEFT_16_H ) ),
    maImgAlgin2h( SVX_RES( IMG_FONTWORK_ALIGN_CENTER_16_H ) ),
    maImgAlgin3h( SVX_RES( IMG_FONTWORK_ALIGN_RIGHT_16_H ) ),
    maImgAlgin4h( SVX_RES( IMG_FONTWORK_ALIGN_WORD_16_H ) ),
    maImgAlgin5h( SVX_RES( IMG_FONTWORK_ALIGN_STRETCH_16_H ) ),
    mbPopupMode( true ),
    mxFrame( rFrame )
{
    implInit();
}

void FontWorkAlignmentWindow::implInit()
{
    SetHelpId( HID_POPUP_FONTWORK_ALIGN );

    bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

    mpMenu = new ToolbarMenu( this, WB_CLIPCHILDREN );
    mpMenu->SetHelpId( HID_POPUP_FONTWORK_ALIGN );
    mpMenu->SetSelectHdl( LINK( this, FontWorkAlignmentWindow, SelectHdl ) );

    mpMenu->appendEntry( 0, String( SVX_RES( STR_ALIGN_LEFT ) ), bHighContrast ? maImgAlgin1h : maImgAlgin1 );
    mpMenu->appendEntry( 1, String( SVX_RES( STR_ALIGN_CENTER ) ), bHighContrast ? maImgAlgin2h : maImgAlgin2 );
    mpMenu->appendEntry( 2, String( SVX_RES( STR_ALIGN_RIGHT ) ), bHighContrast ? maImgAlgin3h : maImgAlgin3 );
    mpMenu->appendEntry( 3, String( SVX_RES( STR_ALIGN_WORD ) ), bHighContrast ? maImgAlgin4h : maImgAlgin4 );
    mpMenu->appendEntry( 4, String( SVX_RES( STR_ALIGN_STRETCH ) ), bHighContrast ? maImgAlgin5h : maImgAlgin5 );

    SetOutputSizePixel( mpMenu->getMenuSize() );
    mpMenu->SetOutputSizePixel( GetOutputSizePixel() );

    mpMenu->Show();

    FreeResource();

    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkAlignment" )));
}

SfxPopupWindow* FontWorkAlignmentWindow::Clone() const
{
    return new FontWorkAlignmentWindow( GetId(), mxFrame );
}

// -----------------------------------------------------------------------

FontWorkAlignmentWindow::~FontWorkAlignmentWindow()
{
    delete mpMenu;
}

// -----------------------------------------------------------------------

void FontWorkAlignmentWindow::implSetAlignment( int nSurface, bool bEnabled )
{
    if( mpMenu )
    {
        int i;
        for( i = 0; i < 5; i++ )
        {
            mpMenu->checkEntry( i, (i == nSurface) && bEnabled );
            mpMenu->enableEntry( i, bEnabled );
        }
    }
}

// -----------------------------------------------------------------------

void FontWorkAlignmentWindow::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    switch( nSID )
    {
        case SID_FONTWORK_ALIGNMENT:
        {
            if( eState == SFX_ITEM_DISABLED )
            {
                implSetAlignment( 0, false );
            }
            else
            {
                const SfxInt32Item* pStateItem = PTR_CAST( SfxInt32Item, pState );
                if( pStateItem )
                    implSetAlignment( pStateItem->GetValue(), true );
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------

void FontWorkAlignmentWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxPopupWindow::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

        mpMenu->appendEntry( 0, String( SVX_RES( STR_ALIGN_LEFT ) ), bHighContrast ? maImgAlgin1h : maImgAlgin1 );
        mpMenu->appendEntry( 1, String( SVX_RES( STR_ALIGN_CENTER ) ), bHighContrast ? maImgAlgin2h : maImgAlgin2 );
        mpMenu->appendEntry( 2, String( SVX_RES( STR_ALIGN_RIGHT ) ), bHighContrast ? maImgAlgin3h : maImgAlgin3 );
        mpMenu->appendEntry( 3, String( SVX_RES( STR_ALIGN_WORD ) ), bHighContrast ? maImgAlgin4h : maImgAlgin4 );
        mpMenu->appendEntry( 4, String( SVX_RES( STR_ALIGN_STRETCH ) ), bHighContrast ? maImgAlgin5h : maImgAlgin5 );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( FontWorkAlignmentWindow, SelectHdl, void *, pControl )
{
    if ( IsInPopupMode() )
        EndPopupMode();

//  SfxDispatcher* pDisp = GetBindings().GetDispatcher();

    sal_Int32 nAlignment = mpMenu->getSelectedEntryId();
    if( nAlignment >= 0 )
    {
        SfxInt32Item    aItem( SID_FONTWORK_ALIGNMENT, nAlignment );
        rtl::OUString   aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkAlignment" ));

        Any a;
        INetURLObject aObj( aCommand );
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = aObj.GetURLPath();
        aItem.QueryValue( a );
        aArgs[0].Value = a;

        SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >(
                                     mxFrame->getController(), UNO_QUERY ),
                                     aCommand,
                                     aArgs );

        implSetAlignment( nAlignment, true );
    }

    return 0;
}

// -----------------------------------------------------------------------

void FontWorkAlignmentWindow::StartSelection()
{
}

// -----------------------------------------------------------------------

BOOL FontWorkAlignmentWindow::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

void FontWorkAlignmentWindow::PopupModeEnd()
{
    if ( IsVisible() )
    {
        mbPopupMode = FALSE;
    }
    SfxPopupWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------

void FontWorkAlignmentWindow::GetFocus (void)
{
    SfxPopupWindow::GetFocus();
    // Grab the focus to the line ends value set so that it can be controlled
    // with the keyboard.
    if( mpMenu )
        mpMenu->GrabFocus();
}

// ========================================================================

FontWorkAlignmentControl::FontWorkAlignmentControl(
    USHORT nSlotId, USHORT nId, ToolBox &rTbx )
: SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

// -----------------------------------------------------------------------

FontWorkAlignmentControl::~FontWorkAlignmentControl()
{
}

// -----------------------------------------------------------------------

SfxPopupWindowType FontWorkAlignmentControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* FontWorkAlignmentControl::CreatePopupWindow()
{
    FontWorkAlignmentWindow* pWin = new FontWorkAlignmentWindow( GetId(), m_xFrame, &GetToolBox() );
    pWin->StartPopupMode( &GetToolBox(), TRUE );
    pWin->StartSelection();
    SetPopupWindow( pWin );
    return pWin;
}

// -----------------------------------------------------------------------

void FontWorkAlignmentControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

// ####################################################################

SFX_IMPL_TOOLBOX_CONTROL( FontWorkCharacterSpacingControl, SfxBoolItem );

FontWorkCharacterSpacingWindow::FontWorkCharacterSpacingWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame ) :

    SfxPopupWindow( nId,
                    rFrame,
                    SVX_RES( RID_SVXFLOAT_FONTWORK_CHARSPACING )),
    mbPopupMode( true ),
    mxFrame( rFrame )
{
    implInit();
}

FontWorkCharacterSpacingWindow::FontWorkCharacterSpacingWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParentWindow ) :

    SfxPopupWindow( nId,
                    rFrame,
                    pParentWindow,
                    SVX_RES( RID_SVXFLOAT_FONTWORK_CHARSPACING )),
    mbPopupMode( true ),
    mxFrame( rFrame )
{
    implInit();
}

void FontWorkCharacterSpacingWindow::implInit()
{
    SetHelpId( HID_POPUP_FONTWORK_CHARSPACE );

    bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

    mpMenu = new ToolbarMenu( this, WB_CLIPCHILDREN );
    mpMenu->SetHelpId( HID_POPUP_FONTWORK_CHARSPACE );
    mpMenu->SetSelectHdl( LINK( this, FontWorkCharacterSpacingWindow, SelectHdl ) );

    mpMenu->appendEntry( 0, String( SVX_RES( STR_CHARS_SPACING_VERY_TIGHT ) ), MIB_RADIOCHECK );
    mpMenu->appendEntry( 1, String( SVX_RES( STR_CHARS_SPACING_TIGHT ) ), MIB_RADIOCHECK );
    mpMenu->appendEntry( 2, String( SVX_RES( STR_CHARS_SPACING_NORMAL ) ), MIB_RADIOCHECK );
    mpMenu->appendEntry( 3, String( SVX_RES( STR_CHARS_SPACING_LOOSE ) ), MIB_RADIOCHECK );
    mpMenu->appendEntry( 4, String( SVX_RES( STR_CHARS_SPACING_VERY_LOOSE ) ), MIB_RADIOCHECK );
    mpMenu->appendEntry( 5, String( SVX_RES( STR_CHARS_SPACING_CUSTOM ) ), MIB_RADIOCHECK );
    mpMenu->appendSeparator();
    mpMenu->appendEntry( 6, String( SVX_RES( STR_CHARS_SPACING_KERN_PAIRS ) ), MIB_CHECKABLE );

    SetOutputSizePixel( mpMenu->getMenuSize() );
    mpMenu->SetOutputSizePixel( GetOutputSizePixel() );

    mpMenu->Show();

    FreeResource();

    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkCharacterSpacing" )));
}

SfxPopupWindow* FontWorkCharacterSpacingWindow::Clone() const
{
    return new FontWorkCharacterSpacingWindow( GetId(), mxFrame );
}

// -----------------------------------------------------------------------

FontWorkCharacterSpacingWindow::~FontWorkCharacterSpacingWindow()
{
    delete mpMenu;
}

// -----------------------------------------------------------------------

void FontWorkCharacterSpacingWindow::implSetCharacterSpacing( sal_Int32 nCharacterSpacing, bool bEnabled )
{
    if( mpMenu )
    {
        sal_Int32 i;
        for ( i = 0; i < 6; i++ )
        {
            mpMenu->checkEntry( i, sal_False );
            mpMenu->enableEntry( i, bEnabled );
        }
        if ( nCharacterSpacing != -1 )
        {
            sal_Int32 nEntry;
            switch( nCharacterSpacing )
            {
                case 80 : nEntry = 0; break;
                case 90 : nEntry = 1; break;
                case 100 : nEntry = 2; break;
                case 120 : nEntry = 3; break;
                case 150 : nEntry = 4; break;
                default : nEntry = 5; break;
            }
            mpMenu->checkEntry( nEntry, bEnabled );
        }
    }
}

void FontWorkCharacterSpacingWindow::implSetKernCharacterPairs( sal_Bool bKernOnOff, bool bEnabled )
{
    if( mpMenu )
    {
        mpMenu->enableEntry( 6, bEnabled );
        mpMenu->checkEntry( 6, bEnabled );
    }
}

// -----------------------------------------------------------------------

void FontWorkCharacterSpacingWindow::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    switch( nSID )
    {
        case SID_FONTWORK_CHARACTER_SPACING:
        {
            if( eState == SFX_ITEM_DISABLED )
                implSetCharacterSpacing( 0, false );
            else
            {
                const SfxInt32Item* pStateItem = PTR_CAST( SfxInt32Item, pState );
                if( pStateItem )
                    implSetCharacterSpacing( pStateItem->GetValue(), true );
            }
            break;
        }
        break;

        case SID_FONTWORK_KERN_CHARACTER_PAIRS :
        {
            if( eState == SFX_ITEM_DISABLED )
                implSetKernCharacterPairs( 0, false );
            else
            {
                const SfxBoolItem* pStateItem = PTR_CAST( SfxBoolItem, pState );
                if( pStateItem )
                    implSetKernCharacterPairs( pStateItem->GetValue(), true );
            }
        }
        break;
    }
}

// -----------------------------------------------------------------------

void FontWorkCharacterSpacingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxPopupWindow::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

        mpMenu->appendEntry( 0, String( SVX_RES( STR_CHARS_SPACING_VERY_TIGHT ) ), MIB_CHECKABLE );
        mpMenu->appendEntry( 1, String( SVX_RES( STR_CHARS_SPACING_TIGHT ) ), MIB_CHECKABLE );
        mpMenu->appendEntry( 2, String( SVX_RES( STR_CHARS_SPACING_NORMAL ) ), MIB_CHECKABLE );
        mpMenu->appendEntry( 3, String( SVX_RES( STR_CHARS_SPACING_LOOSE ) ),  MIB_CHECKABLE );
        mpMenu->appendEntry( 4, String( SVX_RES( STR_CHARS_SPACING_VERY_LOOSE ) ), MIB_CHECKABLE );
        mpMenu->appendEntry( 5, String( SVX_RES( STR_CHARS_SPACING_CUSTOM ) ), MIB_CHECKABLE );
        mpMenu->appendSeparator();
        mpMenu->appendEntry( 6, String( SVX_RES( STR_CHARS_SPACING_KERN_PAIRS ) ), MIB_CHECKABLE );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( FontWorkCharacterSpacingWindow, SelectHdl, void *, pControl )
{
    if ( IsInPopupMode() )
        EndPopupMode();

    sal_Int32 nSelection = mpMenu->getSelectedEntryId();
    sal_Int32 nCharacterSpacing;
    switch( nSelection )
    {
        case 0 : nCharacterSpacing = 80; break;
        case 1 : nCharacterSpacing = 90; break;
        case 2 : nCharacterSpacing = 100; break;
        case 3 : nCharacterSpacing = 120; break;
        case 4 : nCharacterSpacing = 150; break;
        default : nCharacterSpacing = 100; break;
    }
    if ( nSelection == 5 )  // custom spacing
    {
        SfxInt32Item    aItem( SID_FONTWORK_CHARACTER_SPACING, nCharacterSpacing );
        rtl::OUString   aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkCharacterSpacingDialog" ));

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontworkCharacterSpacing" ));
        aItem.QueryValue( a );
        aArgs[0].Value = a;
        SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >(
                                        mxFrame->getController(), UNO_QUERY ),
                                        aCommand,
                                        aArgs );
    }
    else if ( nSelection == 6 ) // KernCharacterPairs
    {
        sal_Bool bOnOff = sal_True;
        SfxBoolItem    aItem( SID_FONTWORK_KERN_CHARACTER_PAIRS, bOnOff );
        rtl::OUString   aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkKernCharacterPairs" ));

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontworkKernCharacterPairs" ));
        aItem.QueryValue( a );
        aArgs[0].Value = a;
        SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >(
                                        mxFrame->getController(), UNO_QUERY ),
                                        aCommand,
                                        aArgs );

        implSetKernCharacterPairs( bOnOff, true );
    }
    else if( nSelection >= 0 )
    {
        SfxInt32Item    aItem( SID_FONTWORK_CHARACTER_SPACING, nCharacterSpacing );
        rtl::OUString   aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkCharacterSpacing" ));

        Any a;
        INetURLObject aObj( aCommand );
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = aObj.GetURLPath();
        aItem.QueryValue( a );
        aArgs[0].Value = a;

        SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >(
                                     mxFrame->getController(), UNO_QUERY ),
                                     aCommand,
                                     aArgs );

        implSetCharacterSpacing( nCharacterSpacing, true );
    }

    return 0;
}

// -----------------------------------------------------------------------

void FontWorkCharacterSpacingWindow::StartSelection()
{
}

// -----------------------------------------------------------------------

BOOL FontWorkCharacterSpacingWindow::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

void FontWorkCharacterSpacingWindow::PopupModeEnd()
{
    if ( IsVisible() )
    {
        mbPopupMode = FALSE;
    }
    SfxPopupWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------

void FontWorkCharacterSpacingWindow::GetFocus (void)
{
    SfxPopupWindow::GetFocus();
    // Grab the focus to the line ends value set so that it can be controlled
    // with the keyboard.
    if( mpMenu )
        mpMenu->GrabFocus();
}

// ========================================================================

FontWorkCharacterSpacingControl::FontWorkCharacterSpacingControl(
    USHORT nSlotId, USHORT nId, ToolBox &rTbx )
: SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

// -----------------------------------------------------------------------

FontWorkCharacterSpacingControl::~FontWorkCharacterSpacingControl()
{
}

// -----------------------------------------------------------------------

SfxPopupWindowType FontWorkCharacterSpacingControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* FontWorkCharacterSpacingControl::CreatePopupWindow()
{
    FontWorkCharacterSpacingWindow* pWin = new FontWorkCharacterSpacingWindow( GetId(), m_xFrame, &GetToolBox() );
    pWin->StartPopupMode( &GetToolBox(), TRUE );
    pWin->StartSelection();
    SetPopupWindow( pWin );
    return pWin;
}

// -----------------------------------------------------------------------

void FontWorkCharacterSpacingControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

// -----------------------------------------------------------------------

FontworkCharacterSpacingDialog::FontworkCharacterSpacingDialog( Window* pParent, sal_Int32 nScale )
:   ModalDialog( pParent, SVX_RES( RID_SVX_MDLG_FONTWORK_CHARSPACING ) ),
    maFLScale( this, SVX_RES( FT_VALUE ) ),
    maMtrScale( this, SVX_RES( MF_VALUE ) ),
    maOKButton( this, SVX_RES( BTN_OK ) ),
    maCancelButton( this, SVX_RES( BTN_CANCEL ) ),
    maHelpButton( this, SVX_RES( BTN_HELP ) )
{
    maMtrScale.SetValue( nScale );
    FreeResource();
}

FontworkCharacterSpacingDialog::~FontworkCharacterSpacingDialog()
{
}

sal_Int32 FontworkCharacterSpacingDialog::getScale() const
{
    return (sal_Int32)maMtrScale.GetValue();
}
