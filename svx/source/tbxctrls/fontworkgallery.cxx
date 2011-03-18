/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <com/sun/star/text/WritingMode.hpp>

#include <vcl/toolbox.hxx>

#include <svl/itempool.hxx>

#include <svtools/toolbarmenu.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/popupmenucontrollerbase.hxx>

#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

#include <svx/fmmodel.hxx>
#include <svx/svxids.hrc>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svx/svdoutl.hxx>

#include "svx/gallery.hxx"
#include <svx/dlgutil.hxx>

#include <svx/fontworkgallery.hxx>
#include "fontworkgallery.hrc"

#include <algorithm>

#include "helpid.hrc"

using ::rtl::OUString;
using ::svtools::ToolbarMenu;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

namespace svx
{

const int nColCount = 4;
const int nLineCount = 4;

/*************************************************************************
|*  Svx3DWin - FloatingWindow
\************************************************************************/
FontWorkGalleryDialog::FontWorkGalleryDialog( SdrView* pSdrView, Window* pParent, sal_uInt16 /*nSID*/ ) :
        ModalDialog( pParent, SVX_RES( RID_SVX_MDLG_FONTWORK_GALLERY ) ),
        maFLFavorites       ( this, SVX_RES( FL_FAVORITES ) ),
        maCtlFavorites      ( this, SVX_RES( CTL_FAVORITES ) ),
        maOKButton          ( this, SVX_RES( BTN_OK ) ),
        maCancelButton      ( this, SVX_RES( BTN_CANCEL ) ),
        maHelpButton        ( this, SVX_RES( BTN_HELP ) ),
        mnThemeId           ( 0xffff ),
        mpSdrView           ( pSdrView ),
        mpModel             ( (FmFormModel*)pSdrView->GetModel() ),
        maStrClickToAddText ( SVX_RES( STR_CLICK_TO_ADD_TEXT ) ),
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
    sal_uIntPtr nFavCount = GalleryExplorer::GetSdrObjCount( nThemeId );

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

    std::vector< Bitmap * >::size_type nFavCount = rFavorites.size();

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

        sal_uInt16 nOutlMode = rOutl.GetMode();
        Size aPaperSize = rOutl.GetPaperSize();
        sal_Bool bUpdateMode = rOutl.GetUpdateMode();
        rOutl.SetUpdateMode(sal_False);
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
    sal_uInt16 nItemId = maCtlFavorites.GetSelectItemId();

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
                OutputDevice* pOutDev = mpSdrView->GetFirstOutputDevice();
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
                    SdrPageView* pPV = mpSdrView->GetSdrPageView();

                    pNewObject->SetLogicRect(aNewObjectRectangle);
                    if ( mppSdrObject )
                    {
                        *mppSdrObject = pNewObject;
                        (*mppSdrObject)->SetModel( mpDestModel );
                    }
                    else if( pPV )
                    {
                            mpSdrView->InsertObjectAtView( pNewObject, *pPV );
    //                      changeText( PTR_CAST( SdrTextObj, pNewObject ) );
                    }
                }
            }
        }

        delete pModel;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( FontWorkGalleryDialog, ClickOKHdl, void*, EMPTYARG )
{
    insertSelectedFontwork();
    EndDialog( true );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( FontWorkGalleryDialog, DoubleClickFavoriteHdl, void*, EMPTYARG )
{
    insertSelectedFontwork();
    EndDialog( true );
    return( 0L );
}

//------------------------------------------------------------------------

SFX_IMPL_TOOLBOX_CONTROL( FontWorkShapeTypeControl, SfxStringItem );
FontWorkShapeTypeControl::FontWorkShapeTypeControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx )
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

void FontWorkShapeTypeControl::Select( sal_Bool )
{

}

// ========================================================================
// FontWorkAlignmentWindow
// ========================================================================

class FontWorkAlignmentWindow : public ToolbarMenu
{
public:
    FontWorkAlignmentWindow( svt::ToolboxController& rController, const Reference< XFrame >& rFrame, Window* pParentWindow );

    virtual void SAL_CALL statusChanged( const frame::FeatureStateEvent& Event ) throw ( RuntimeException );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );

private:
    svt::ToolboxController& mrController;

    Image maImgAlgin1;
    Image maImgAlgin2;
    Image maImgAlgin3;
    Image maImgAlgin4;
    Image maImgAlgin5;

    const rtl::OUString msFontworkAlignment;

    DECL_LINK( SelectHdl, void * );

    void    implSetAlignment( int nAlignmentMode, bool bEnabled );
};

FontWorkAlignmentWindow::FontWorkAlignmentWindow( svt::ToolboxController& rController, const Reference< XFrame >& rFrame, Window* pParentWindow )
: ToolbarMenu( rFrame, pParentWindow, SVX_RES( RID_SVXFLOAT_FONTWORK_ALIGNMENT ))
, mrController( rController )
, maImgAlgin1( SVX_RES( IMG_FONTWORK_ALIGN_LEFT_16    ) )
, maImgAlgin2( SVX_RES( IMG_FONTWORK_ALIGN_CENTER_16  ) )
, maImgAlgin3( SVX_RES( IMG_FONTWORK_ALIGN_RIGHT_16   ) )
, maImgAlgin4( SVX_RES( IMG_FONTWORK_ALIGN_WORD_16    ) )
, maImgAlgin5( SVX_RES( IMG_FONTWORK_ALIGN_STRETCH_16 ) )
, msFontworkAlignment( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkAlignment" ) )
{
    SetHelpId( HID_POPUP_FONTWORK_ALIGN );
    SetSelectHdl( LINK( this, FontWorkAlignmentWindow, SelectHdl ) );

    appendEntry( 0, String( SVX_RES( STR_ALIGN_LEFT    ) ), maImgAlgin1 );
    appendEntry( 1, String( SVX_RES( STR_ALIGN_CENTER  ) ), maImgAlgin2 );
    appendEntry( 2, String( SVX_RES( STR_ALIGN_RIGHT   ) ), maImgAlgin3 );
    appendEntry( 3, String( SVX_RES( STR_ALIGN_WORD    ) ), maImgAlgin4 );
    appendEntry( 4, String( SVX_RES( STR_ALIGN_STRETCH ) ), maImgAlgin5 );

    SetOutputSizePixel( getMenuSize() );

    FreeResource();

    AddStatusListener( msFontworkAlignment );
}

// -----------------------------------------------------------------------

void FontWorkAlignmentWindow::implSetAlignment( int nSurface, bool bEnabled )
{
    int i;
    for( i = 0; i < 5; i++ )
    {
        checkEntry( i, (i == nSurface) && bEnabled );
        enableEntry( i, bEnabled );
    }
}

// -----------------------------------------------------------------------

void SAL_CALL FontWorkAlignmentWindow::statusChanged( const frame::FeatureStateEvent& Event ) throw ( RuntimeException )
{
    if( Event.FeatureURL.Main.equals( msFontworkAlignment ) )
    {
        if( !Event.IsEnabled )
        {
            implSetAlignment( 0, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetAlignment( nValue, true );
        }
    }
}

// -----------------------------------------------------------------------

void FontWorkAlignmentWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolbarMenu::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        appendEntry( 0, String( SVX_RES( STR_ALIGN_LEFT    ) ), maImgAlgin1 );
        appendEntry( 1, String( SVX_RES( STR_ALIGN_CENTER  ) ), maImgAlgin2 );
        appendEntry( 2, String( SVX_RES( STR_ALIGN_RIGHT   ) ), maImgAlgin3 );
        appendEntry( 3, String( SVX_RES( STR_ALIGN_WORD    ) ), maImgAlgin4 );
        appendEntry( 4, String( SVX_RES( STR_ALIGN_STRETCH ) ), maImgAlgin5 );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( FontWorkAlignmentWindow, SelectHdl, void *, EMPTYARG )
{
    if ( IsInPopupMode() )
        EndPopupMode();

    sal_Int32 nAlignment = getSelectedEntryId();
    if( nAlignment >= 0 )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = msFontworkAlignment.copy(5);
        aArgs[0].Value <<= (sal_Int32)nAlignment;

        mrController.dispatchCommand( msFontworkAlignment, aArgs );

        implSetAlignment( nAlignment, true );
    }

    return 0;
}

// ========================================================================
// FontWorkAlignmentControl
// ========================================================================

class FontWorkAlignmentControl : public svt::PopupWindowController
{
public:
    FontWorkAlignmentControl( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager );

    virtual ::Window* createPopupWindow( ::Window* pParent );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    using  svt::PopupWindowController::createPopupWindow;
};


// -----------------------------------------------------------------------

FontWorkAlignmentControl::FontWorkAlignmentControl( const Reference< lang::XMultiServiceFactory >& rServiceManager )
: svt::PopupWindowController( rServiceManager, Reference< frame::XFrame >(), OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkAlignment" ) ) )
{
}

// -----------------------------------------------------------------------

::Window* FontWorkAlignmentControl::createPopupWindow( ::Window* pParent )
{
    return new FontWorkAlignmentWindow( *this, m_xFrame, pParent );
}

// -----------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------

OUString SAL_CALL FontWorkAlignmentControl_getImplementationName()
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svx.FontworkAlignmentController" ));
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL FontWorkAlignmentControl_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ));
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL SAL_CALL FontWorkAlignmentControl_createInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( RuntimeException )
{
    return *new FontWorkAlignmentControl( rSMgr );
}

// --------------------------------------------------------------------

OUString SAL_CALL FontWorkAlignmentControl::getImplementationName(  ) throw (RuntimeException)
{
    return FontWorkAlignmentControl_getImplementationName();
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL FontWorkAlignmentControl::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return FontWorkAlignmentControl_getSupportedServiceNames();
}


// ####################################################################

class FontWorkCharacterSpacingWindow : public ToolbarMenu
{
public:
    FontWorkCharacterSpacingWindow( svt::ToolboxController& rController, const Reference< XFrame >& rFrame, Window* pParentWindow );

    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );
private:
    svt::ToolboxController& mrController;

    const rtl::OUString msFontworkCharacterSpacing;
    const rtl::OUString msFontworkKernCharacterPairs;

    DECL_LINK( SelectHdl, void * );

    void    implSetCharacterSpacing( sal_Int32 nCharacterSpacing, bool bEnabled );
    void    implSetKernCharacterPairs( sal_Bool bKernOnOff, bool bEnabled );

};

// -----------------------------------------------------------------------

FontWorkCharacterSpacingWindow::FontWorkCharacterSpacingWindow( svt::ToolboxController& rController, const Reference< XFrame >& rFrame, Window* pParentWindow )
: ToolbarMenu( rFrame, pParentWindow, SVX_RES( RID_SVXFLOAT_FONTWORK_CHARSPACING ))
, mrController( rController )
, msFontworkCharacterSpacing( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkCharacterSpacing" ) )
, msFontworkKernCharacterPairs( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkKernCharacterPairs" ) )
{
    SetHelpId( HID_POPUP_FONTWORK_CHARSPACE );
    SetSelectHdl( LINK( this, FontWorkCharacterSpacingWindow, SelectHdl ) );

    appendEntry( 0, String( SVX_RES( STR_CHARS_SPACING_VERY_TIGHT ) ), MIB_RADIOCHECK );
    appendEntry( 1, String( SVX_RES( STR_CHARS_SPACING_TIGHT      ) ), MIB_RADIOCHECK );
    appendEntry( 2, String( SVX_RES( STR_CHARS_SPACING_NORMAL     ) ), MIB_RADIOCHECK );
    appendEntry( 3, String( SVX_RES( STR_CHARS_SPACING_LOOSE      ) ), MIB_RADIOCHECK );
    appendEntry( 4, String( SVX_RES( STR_CHARS_SPACING_VERY_LOOSE ) ), MIB_RADIOCHECK );
    appendEntry( 5, String( SVX_RES( STR_CHARS_SPACING_CUSTOM     ) ), MIB_RADIOCHECK );
    appendSeparator();
    appendEntry( 6, String( SVX_RES( STR_CHARS_SPACING_KERN_PAIRS ) ), MIB_CHECKABLE  );

    SetOutputSizePixel( getMenuSize() );

    FreeResource();

    AddStatusListener( msFontworkCharacterSpacing );
    AddStatusListener( msFontworkKernCharacterPairs );
}

// -----------------------------------------------------------------------

void FontWorkCharacterSpacingWindow::implSetCharacterSpacing( sal_Int32 nCharacterSpacing, bool bEnabled )
{
    sal_Int32 i;
    for ( i = 0; i < 6; i++ )
    {
        checkEntry( i, sal_False );
        enableEntry( i, bEnabled );
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
        checkEntry( nEntry, bEnabled );
    }
}

// -----------------------------------------------------------------------

void FontWorkCharacterSpacingWindow::implSetKernCharacterPairs( sal_Bool, bool bEnabled )
{
    enableEntry( 6, bEnabled );
    checkEntry( 6, bEnabled );
}

// -----------------------------------------------------------------------

void SAL_CALL FontWorkCharacterSpacingWindow::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException )
{
    if( Event.FeatureURL.Main.equals( msFontworkCharacterSpacing ) )
    {
        if( !Event.IsEnabled )
        {
            implSetCharacterSpacing( 0, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetCharacterSpacing( nValue, true );
        }
    }
    else if( Event.FeatureURL.Main.equals( msFontworkKernCharacterPairs ) )
    {
        if( !Event.IsEnabled )
        {
            implSetKernCharacterPairs( 0, false );
        }
        else
        {
            sal_Bool bValue = sal_False;
            if( Event.State >>= bValue )
                implSetKernCharacterPairs( bValue, true );
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( FontWorkCharacterSpacingWindow, SelectHdl, void *, EMPTYARG )
{
    if ( IsInPopupMode() )
        EndPopupMode();

    sal_Int32 nSelection = getSelectedEntryId();
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
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = msFontworkCharacterSpacing.copy(5);
        aArgs[0].Value <<= (sal_Int32)nCharacterSpacing;

        mrController.dispatchCommand( OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkCharacterSpacingDialog" )), aArgs );
    }
    else if ( nSelection == 6 ) // KernCharacterPairs
    {
        rtl::OUString   aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkKernCharacterPairs" ));

        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = msFontworkKernCharacterPairs.copy(5);
        aArgs[0].Value <<= (sal_Bool) sal_True;

        mrController.dispatchCommand( msFontworkKernCharacterPairs, aArgs );

        implSetKernCharacterPairs( sal_True, true );
    }
    else if( nSelection >= 0 )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = msFontworkCharacterSpacing.copy(5);
        aArgs[0].Value <<=( sal_Int32)nCharacterSpacing;

        mrController.dispatchCommand( msFontworkCharacterSpacing,  aArgs );

        implSetCharacterSpacing( nCharacterSpacing, true );
    }

    return 0;
}

// ========================================================================
// FontWorkCharacterSpacingControl
// ========================================================================

class FontWorkCharacterSpacingControl : public svt::PopupWindowController
{
public:
    FontWorkCharacterSpacingControl( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager );

    virtual ::Window* createPopupWindow( ::Window* pParent );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    using svt::PopupWindowController::createPopupWindow;
};


FontWorkCharacterSpacingControl::FontWorkCharacterSpacingControl( const Reference< lang::XMultiServiceFactory >& rServiceManager )
: svt::PopupWindowController( rServiceManager, Reference< frame::XFrame >(), OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontworkCharacterSpacingFloater" ) ) )
{
}

// -----------------------------------------------------------------------

::Window* FontWorkCharacterSpacingControl::createPopupWindow( ::Window* pParent )
{
    return new FontWorkCharacterSpacingWindow( *this, m_xFrame, pParent );
}

// -----------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------

OUString SAL_CALL FontWorkCharacterSpacingControl_getImplementationName()
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svx.FontworkCharacterSpacingController" ));
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL FontWorkCharacterSpacingControl_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ));
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL SAL_CALL FontWorkCharacterSpacingControl_createInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( RuntimeException )
{
    return *new FontWorkCharacterSpacingControl( rSMgr );
}

// --------------------------------------------------------------------

OUString SAL_CALL FontWorkCharacterSpacingControl::getImplementationName(  ) throw (RuntimeException)
{
    return FontWorkCharacterSpacingControl_getImplementationName();
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL FontWorkCharacterSpacingControl::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return FontWorkCharacterSpacingControl_getSupportedServiceNames();
}

// ========================================================================
// FontworkCharacterSpacingDialog
// ========================================================================

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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
