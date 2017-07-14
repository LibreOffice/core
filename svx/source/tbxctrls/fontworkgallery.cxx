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


#include <com/sun/star/text/WritingMode.hpp>

#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>

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
#include "coreservices.hxx"

#include <algorithm>
#include <memory>

#include "helpid.hrc"
#include "bitmaps.hlst"

using ::svtools::ToolbarMenu;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace svx
{

const int nColCount = 4;
const int nLineCount = 4;

FontWorkGalleryDialog::FontWorkGalleryDialog( SdrView* pSdrView, vcl::Window* pParent ) :
        ModalDialog(pParent, "FontworkGalleryDialog", "svx/ui/fontworkgallerydialog.ui" ),
        mnThemeId           ( 0xffff ),
        mpSdrView           ( pSdrView ),
        mppSdrObject        ( nullptr ),
        mpDestModel         ( nullptr )
{
    get(mpOKButton, "ok");
    get(mpCtlFavorites, "ctlFavorites");
    Size aSize(LogicToPixel(Size(200, 200), MapUnit::MapAppFont));
    mpCtlFavorites->set_width_request(aSize.Width());
    mpCtlFavorites->set_height_request(aSize.Height());

    mpCtlFavorites->SetDoubleClickHdl( LINK( this, FontWorkGalleryDialog, DoubleClickFavoriteHdl ) );
    mpOKButton->SetClickHdl( LINK( this, FontWorkGalleryDialog, ClickOKHdl ) );

    mpCtlFavorites->SetColCount( nColCount );
    mpCtlFavorites->SetLineCount( nLineCount );
    mpCtlFavorites->SetExtraSpacing( 3 );

    initFavorites( GALLERY_THEME_FONTWORK );
    fillFavorites( GALLERY_THEME_FONTWORK );
}

FontWorkGalleryDialog::~FontWorkGalleryDialog()
{
    disposeOnce();
}

void FontWorkGalleryDialog::dispose()
{
    mpCtlFavorites.clear();
    mpOKButton.clear();
    ModalDialog::dispose();
}

void FontWorkGalleryDialog::initFavorites(sal_uInt16 nThemeId)
{
    // the favorites are read via the gallery
    sal_uInt32 nFavCount = GalleryExplorer::GetSdrObjCount( nThemeId );

    // lock gallery theme
    GalleryExplorer::BeginLocking(nThemeId);

    sal_uInt32 nModelPos;
    FmFormModel *pModel = nullptr;

    for( nModelPos = 0; nModelPos < nFavCount; nModelPos++ )
    {
        BitmapEx aThumb;

        if (GalleryExplorer::GetSdrObj(nThemeId, nModelPos, pModel, &aThumb) && !!aThumb)
        {
            ScopedVclPtrInstance< VirtualDevice > pVDev;
            const Point aNull(0, 0);

            if (GetDPIScaleFactor() > 1)
                aThumb.Scale(GetDPIScaleFactor(), GetDPIScaleFactor());

            const Size aSize(aThumb.GetSizePixel());

            pVDev->SetOutputSizePixel(aSize);

            static const sal_uInt32 nLen(8);
            static const Color aW(COL_WHITE);
            static const Color aG(0xef, 0xef, 0xef);

            pVDev->DrawCheckered(aNull, aSize, nLen, aW, aG);

            pVDev->DrawBitmapEx(aNull, aThumb);
            maFavoritesHorizontal.push_back(pVDev->GetBitmap(aNull, aSize));
        }
    }

    // release gallery theme
    GalleryExplorer::EndLocking(nThemeId);
}

void FontWorkGalleryDialog::fillFavorites(sal_uInt16 nThemeId)
{
    mnThemeId = nThemeId;

    Size aThumbSize( mpCtlFavorites->GetSizePixel() );
    aThumbSize.Width() /= nColCount;
    aThumbSize.Height() /= nLineCount;
    aThumbSize.Width() -= 12;
    aThumbSize.Height() -= 12;

    std::vector< Bitmap * >::size_type nFavCount = maFavoritesHorizontal.size();

    // ValueSet favorites
    if( nFavCount > (nColCount * nLineCount) )
    {
        WinBits nWinBits = mpCtlFavorites->GetStyle();
        nWinBits |= WB_VSCROLL;
        mpCtlFavorites->SetStyle( nWinBits );
    }

    mpCtlFavorites->Clear();

    for( std::vector<Bitmap *>::size_type nFavorite = 1; nFavorite <= nFavCount; nFavorite++ )
    {
        OUString aStr(SvxResId(RID_SVXFLOAT3D_FAVORITE));
        aStr += " ";
        aStr += OUString::number(nFavorite);
        Image aThumbImage( maFavoritesHorizontal[nFavorite-1] );
        mpCtlFavorites->InsertItem( (sal_uInt16)nFavorite, aThumbImage, aStr );
    }
}

void FontWorkGalleryDialog::SetSdrObjectRef( SdrObject** ppSdrObject, SdrModel* pModel )
{
    mppSdrObject = ppSdrObject;
    mpDestModel = pModel;
}

void FontWorkGalleryDialog::insertSelectedFontwork()
{
    sal_uInt16 nItemId = mpCtlFavorites->GetSelectItemId();

    if( nItemId > 0 )
    {
        std::unique_ptr<FmFormModel> pModel(new FmFormModel());
        pModel->GetItemPool().FreezeIdRanges();

        if( GalleryExplorer::GetSdrObj( mnThemeId, nItemId-1, pModel.get() ) )
        {
            SdrPage* pPage = pModel->GetPage(0);
            if( pPage && pPage->GetObjCount() )
            {
                SdrObject* pNewObject = pPage->GetObj(0)->Clone();

                // center shape on current view
                OutputDevice* pOutDev = mpSdrView->GetFirstOutputDevice();
                if( pOutDev )
                {
                    tools::Rectangle aObjRect( pNewObject->GetLogicRect() );
                    tools::Rectangle aVisArea = pOutDev->PixelToLogic(tools::Rectangle(Point(0,0), pOutDev->GetOutputSizePixel()));
                    Point aPagePos = aVisArea.Center();
                    aPagePos.X() -= aObjRect.GetWidth() / 2;
                    aPagePos.Y() -= aObjRect.GetHeight() / 2;
                    tools::Rectangle aNewObjectRectangle(aPagePos, aObjRect.GetSize());
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
                    }
                }
            }
        }
    }
}


IMPL_LINK_NOARG(FontWorkGalleryDialog, ClickOKHdl, Button*, void)
{
    insertSelectedFontwork();
    EndDialog( RET_OK );
}


IMPL_LINK_NOARG(FontWorkGalleryDialog, DoubleClickFavoriteHdl, ValueSet*, void)
{
    insertSelectedFontwork();
    EndDialog( RET_OK );
}


class FontworkAlignmentWindow : public ToolbarMenu
{
public:
    FontworkAlignmentWindow( svt::ToolboxController& rController, vcl::Window* pParentWindow );

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override;

private:
    svt::ToolboxController& mrController;

    Image maImgAlgin1;
    Image maImgAlgin2;
    Image maImgAlgin3;
    Image maImgAlgin4;
    Image maImgAlgin5;

    const OUString msFontworkAlignment;

    DECL_LINK( SelectHdl, ToolbarMenu*, void );

    void    implSetAlignment( int nAlignmentMode, bool bEnabled );
};

FontworkAlignmentWindow::FontworkAlignmentWindow(svt::ToolboxController& rController, vcl::Window* pParentWindow)
    : ToolbarMenu(rController.getFrameInterface(), pParentWindow, WB_STDPOPUP)
    , mrController(rController)
    , maImgAlgin1(BitmapEx(RID_SVXBMP_FONTWORK_ALIGN_LEFT))
    , maImgAlgin2(BitmapEx(RID_SVXBMP_FONTWORK_ALIGN_CENTER))
    , maImgAlgin3(BitmapEx(RID_SVXBMP_FONTWORK_ALIGN_RIGHT))
    , maImgAlgin4(BitmapEx(RID_SVXBMP_FONTWORK_ALIGN_WORD))
    , maImgAlgin5(BitmapEx(RID_SVXBMP_FONTWORK_ALIGN_STRETCH))
    , msFontworkAlignment(".uno:FontworkAlignment")
{
    SetSelectHdl( LINK( this, FontworkAlignmentWindow, SelectHdl ) );

    appendEntry(0, SvxResId(RID_SVXSTR_ALIGN_LEFT), maImgAlgin1);
    appendEntry(1, SvxResId(RID_SVXSTR_ALIGN_CENTER), maImgAlgin2);
    appendEntry(2, SvxResId(RID_SVXSTR_ALIGN_RIGHT), maImgAlgin3);
    appendEntry(3, SvxResId(RID_SVXSTR_ALIGN_WORD), maImgAlgin4);
    appendEntry(4, SvxResId(RID_SVXSTR_ALIGN_STRETCH), maImgAlgin5);

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( msFontworkAlignment );
}

void FontworkAlignmentWindow::implSetAlignment( int nSurface, bool bEnabled )
{
    int i;
    for( i = 0; i < 5; i++ )
    {
        checkEntry( i, (i == nSurface) && bEnabled );
        enableEntry( i, bEnabled );
    }
}

void FontworkAlignmentWindow::statusChanged( const css::frame::FeatureStateEvent& Event )
{
    if( Event.FeatureURL.Main == msFontworkAlignment )
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

IMPL_LINK_NOARG(FontworkAlignmentWindow, SelectHdl, ToolbarMenu*, void)
{
    if ( IsInPopupMode() )
        EndPopupMode();

    sal_Int32 nAlignment = getSelectedEntryId();
    if( nAlignment >= 0 )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = msFontworkAlignment.copy(5);
        aArgs[0].Value <<= nAlignment;

        mrController.dispatchCommand( msFontworkAlignment, aArgs );

        implSetAlignment( nAlignment, true );
    }
}

class FontworkAlignmentControl : public svt::PopupWindowController
{
public:
    explicit FontworkAlignmentControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    using  svt::PopupWindowController::createPopupWindow;
};


FontworkAlignmentControl::FontworkAlignmentControl( const Reference< XComponentContext >& rxContext )
: svt::PopupWindowController( rxContext, Reference< css::frame::XFrame >(), ".uno:FontworkAlignment" )
{
}


VclPtr<vcl::Window> FontworkAlignmentControl::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<FontworkAlignmentWindow>::Create( *this, pParent );
}

// XInitialization
void SAL_CALL FontworkAlignmentControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString SAL_CALL FontworkAlignmentControl_getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.FontworkAlignmentController" );
}


Sequence< OUString > SAL_CALL FontworkAlignmentControl_getSupportedServiceNames()
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


Reference< XInterface > SAL_CALL SAL_CALL FontworkAlignmentControl_createInstance( const Reference< XMultiServiceFactory >& rSMgr )
{
    return *new FontworkAlignmentControl( comphelper::getComponentContext(rSMgr) );
}


OUString SAL_CALL FontworkAlignmentControl::getImplementationName(  )
{
    return FontworkAlignmentControl_getImplementationName();
}


Sequence< OUString > SAL_CALL FontworkAlignmentControl::getSupportedServiceNames(  )
{
    return FontworkAlignmentControl_getSupportedServiceNames();
}

class FontworkCharacterSpacingWindow : public ToolbarMenu
{
public:
    FontworkCharacterSpacingWindow( svt::ToolboxController& rController, vcl::Window* pParentWindow );

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override;
private:
    svt::ToolboxController& mrController;

    const OUString msFontworkCharacterSpacing;
    const OUString msFontworkKernCharacterPairs;

    DECL_LINK( SelectHdl, ToolbarMenu*, void );

    void    implSetCharacterSpacing( sal_Int32 nCharacterSpacing, bool bEnabled );
    void    implSetKernCharacterPairs( bool bEnabled );

};

FontworkCharacterSpacingWindow::FontworkCharacterSpacingWindow(svt::ToolboxController& rController, vcl::Window* pParentWindow)
    : ToolbarMenu(rController.getFrameInterface(), pParentWindow, WB_STDPOPUP)
    , mrController(rController)
    , msFontworkCharacterSpacing(".uno:FontworkCharacterSpacing")
    , msFontworkKernCharacterPairs(".uno:FontworkKernCharacterPairs")
{
    SetSelectHdl( LINK( this, FontworkCharacterSpacingWindow, SelectHdl ) );

    appendEntry(0, SvxResId(RID_SVXSTR_CHARS_SPACING_VERY_TIGHT), MenuItemBits::RADIOCHECK);
    appendEntry(1, SvxResId(RID_SVXSTR_CHARS_SPACING_TIGHT), MenuItemBits::RADIOCHECK);
    appendEntry(2, SvxResId(RID_SVXSTR_CHARS_SPACING_NORMAL), MenuItemBits::RADIOCHECK);
    appendEntry(3, SvxResId(RID_SVXSTR_CHARS_SPACING_LOOSE), MenuItemBits::RADIOCHECK);
    appendEntry(4, SvxResId(RID_SVXSTR_CHARS_SPACING_VERY_LOOSE), MenuItemBits::RADIOCHECK);
    appendEntry(5, SvxResId(RID_SVXSTR_CHARS_SPACING_CUSTOM), MenuItemBits::RADIOCHECK);
    appendSeparator();
    appendEntry(6, SvxResId(RID_SVXSTR_CHARS_SPACING_KERN_PAIRS), MenuItemBits::CHECKABLE);

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( msFontworkCharacterSpacing );
    AddStatusListener( msFontworkKernCharacterPairs );
}

void FontworkCharacterSpacingWindow::implSetCharacterSpacing( sal_Int32 nCharacterSpacing, bool bEnabled )
{
    sal_Int32 i;
    for ( i = 0; i < 6; i++ )
    {
        checkEntry( i, false );
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


void FontworkCharacterSpacingWindow::implSetKernCharacterPairs( bool bEnabled )
{
    enableEntry( 6, bEnabled );
    checkEntry( 6, bEnabled );
}


void FontworkCharacterSpacingWindow::statusChanged( const css::frame::FeatureStateEvent& Event )
{
    if( Event.FeatureURL.Main == msFontworkCharacterSpacing )
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
    else if( Event.FeatureURL.Main == msFontworkKernCharacterPairs )
    {
        if( !Event.IsEnabled )
        {
            implSetKernCharacterPairs( false );
        }
        else
        {
            bool bValue = false;
            if( Event.State >>= bValue )
                implSetKernCharacterPairs( true );
        }
    }
}


IMPL_LINK_NOARG(FontworkCharacterSpacingWindow, SelectHdl,ToolbarMenu*, void)
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
        aArgs[0].Value <<= nCharacterSpacing;

        mrController.dispatchCommand( ".uno:FontworkCharacterSpacingDialog", aArgs );
    }
    else if ( nSelection == 6 ) // KernCharacterPairs
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = msFontworkKernCharacterPairs.copy(5);
        aArgs[0].Value <<= true;

        mrController.dispatchCommand( msFontworkKernCharacterPairs, aArgs );

        implSetKernCharacterPairs( true );
    }
    else if( nSelection >= 0 )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = msFontworkCharacterSpacing.copy(5);
        aArgs[0].Value <<= nCharacterSpacing;

        mrController.dispatchCommand( msFontworkCharacterSpacing,  aArgs );

        implSetCharacterSpacing( nCharacterSpacing, true );
    }
}

class FontworkCharacterSpacingControl : public svt::PopupWindowController
{
public:
    explicit FontworkCharacterSpacingControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    using svt::PopupWindowController::createPopupWindow;
};


FontworkCharacterSpacingControl::FontworkCharacterSpacingControl( const Reference< XComponentContext >& rxContext )
: svt::PopupWindowController( rxContext, Reference< css::frame::XFrame >(), ".uno:FontworkCharacterSpacingFloater" )
{
}


VclPtr<vcl::Window> FontworkCharacterSpacingControl::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<FontworkCharacterSpacingWindow>::Create( *this, pParent );
}

// XInitialization
void SAL_CALL FontworkCharacterSpacingControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString SAL_CALL FontworkCharacterSpacingControl_getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.FontworkCharacterSpacingController" );
}


Sequence< OUString > SAL_CALL FontworkCharacterSpacingControl_getSupportedServiceNames()
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


Reference< XInterface > SAL_CALL SAL_CALL FontworkCharacterSpacingControl_createInstance( const Reference< XMultiServiceFactory >& rSMgr )
{
    return *new FontworkCharacterSpacingControl( comphelper::getComponentContext(rSMgr) );
}


OUString SAL_CALL FontworkCharacterSpacingControl::getImplementationName(  )
{
    return FontworkCharacterSpacingControl_getImplementationName();
}


Sequence< OUString > SAL_CALL FontworkCharacterSpacingControl::getSupportedServiceNames(  )
{
    return FontworkCharacterSpacingControl_getSupportedServiceNames();
}

FontworkCharacterSpacingDialog::FontworkCharacterSpacingDialog( vcl::Window* pParent, sal_Int32 nScale )
:   ModalDialog( pParent, "FontworkSpacingDialog" , "svx/ui/fontworkspacingdialog.ui" )
{
    get(m_pMtrScale, "entry");
    m_pMtrScale->SetValue( nScale );
}

FontworkCharacterSpacingDialog::~FontworkCharacterSpacingDialog()
{
    disposeOnce();
}

void FontworkCharacterSpacingDialog::dispose()
{
    m_pMtrScale.clear();
    ModalDialog::dispose();
}

sal_Int32 FontworkCharacterSpacingDialog::getScale() const
{
    return (sal_Int32)m_pMtrScale->GetValue();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
