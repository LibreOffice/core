/*************************************************************************
 *
 *  $RCSfile: fontworkgallery.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-04-02 14:15:04 $
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

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SVX_FMMODEL_HXX
#include <fmmodel.hxx>
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

#include "fontworkgallery.hxx"
#include "fontworkgallery.hrc"

#include <algorithm>

using namespace svx;

const int nColCount = 4;
const int nLineCount = 4;

/*************************************************************************
|*  Svx3DWin - FloatingWindow
\************************************************************************/
FontWorkGalleryDialog::FontWorkGalleryDialog( SdrView* pSdrView, Window* pParent, sal_uInt16 nSID ) :
        ModalDialog( pParent, SVX_RES( RID_SVX_MDLG_FONTWORK_GALLERY ) ),
        maCtlFavorites      ( this, SVX_RES( CTL_FAVORITES ) ),
        maFLFavorites       ( this, SVX_RES( FL_FAVORITES ) ),
        maBtnLTR            ( this, SVX_RES( BTN_LTR ) ),
        maBtnTTB            ( this, SVX_RES( BTN_TTB ) ),
        maOKButton          ( this, SVX_RES( BTN_OK ) ),
        maCancelButton      ( this, SVX_RES( BTN_CANCEL ) ),
        mnThemeId           ( -1 ),
        maStrClickToAddText ( SVX_RES( STR_CLICK_TO_ADD_TEXT ) ),
        mpSdrView           ( pSdrView ),
        mpModel             ( (FmFormModel*)pSdrView->GetModel() )
{
    FreeResource();

    maCtlFavorites.SetDoubleClickHdl( LINK( this, FontWorkGalleryDialog, DoubleClickFavoriteHdl ) );
    maOKButton.SetClickHdl( LINK( this, FontWorkGalleryDialog, ClickOKHdl ) );
    maBtnLTR.SetClickHdl( LINK( this, FontWorkGalleryDialog, ClickTextDirectionHdl ) );
    maBtnTTB.SetClickHdl( LINK( this, FontWorkGalleryDialog, ClickTextDirectionHdl ) );

    maCtlFavorites.SetColCount( nColCount );
    maCtlFavorites.SetLineCount( nLineCount );
    maCtlFavorites.SetExtraSpacing( 3 );

    initfavorites( GALLERY_THEME_FONTWORK, maFavoritesHorizontal );
    initfavorites( GALLERY_THEME_FONTWORK_VERTICAL, maFavoritesVertical );

    ImageButton* p = &maBtnLTR;
    switch( nSID )
    {
//  case SID_DRAW_FONTWORK:
    case SID_DRAW_FONTWORK_VERTICAL:
        p = &maBtnTTB;
        break;
    case SID_FONTWORK_GALLERY_FLOATER:
        {
            const SvxFrameDirectionItem* pItem = (const SvxFrameDirectionItem*)mpModel->GetItemPool().GetPoolDefaultItem( EE_PARA_WRITINGDIR );
            if( pItem && pItem->GetValue() == ::com::sun::star::text::WritingMode_TB_RL )
            {
                p = &maBtnTTB;
            }
            break;
        }
    }

    ClickTextDirectionHdl( p );
}

static void delete_bitmap( Bitmap* p ) { delete p; }

// -----------------------------------------------------------------------
FontWorkGalleryDialog::~FontWorkGalleryDialog()
{
    std::for_each( maFavoritesVertical.begin(), maFavoritesVertical.end(), delete_bitmap );
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
                    Point aPagePos = aVisArea.Center();
                    aPagePos.X() -= aObjRect.GetWidth() / 2;
                    aPagePos.Y() -= aObjRect.GetHeight() / 2;
                    Rectangle aNewObjectRectangle(aPagePos, aObjRect.GetSize());
                    SdrPageView* pPV = mpSdrView->GetPageViewPvNum(0);

                    pNewObject->SetLogicRect(aNewObjectRectangle);

                    if( pPV )
                    {
                        mpSdrView->InsertObject( pNewObject, *pPV );
                        changeText( PTR_CAST( SdrTextObj, pNewObject ) );
                    }
                }
            }
        }

        delete pModel;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( FontWorkGalleryDialog, ClickTextDirectionHdl, ImageButton *, pButton )
{
    bool bHorizontal = pButton == &maBtnLTR;

    maBtnLTR.SetPressed( bHorizontal );
    maBtnTTB.SetPressed( !bHorizontal );

    if( (mnThemeId == (sal_uInt16)-1) ||
        (bHorizontal && (mnThemeId == GALLERY_THEME_FONTWORK_VERTICAL)) ||
        (!bHorizontal && (mnThemeId == GALLERY_THEME_FONTWORK)) )
    {
        if( bHorizontal )
        {
            fillFavorites( GALLERY_THEME_FONTWORK, maFavoritesHorizontal );
        }
        else
        {
            fillFavorites( GALLERY_THEME_FONTWORK_VERTICAL, maFavoritesVertical );
        }
    }
    return 0;
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
