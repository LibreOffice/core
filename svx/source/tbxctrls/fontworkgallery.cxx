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
#include <vcl/virdev.hxx>

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
#include <svx/strings.hrc>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svx/svdoutl.hxx>

#include <svx/gallery.hxx>
#include <svx/dlgutil.hxx>
#include <svx/fontworkgallery.hxx>

#include <algorithm>
#include <memory>

#include <bitmaps.hlst>

using ::svtools::ToolbarMenu;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace svx
{

const int nColCount = 4;
const int nLineCount = 4;

FontWorkGalleryDialog::FontWorkGalleryDialog(weld::Window* pParent, SdrView& rSdrView)
    : GenericDialogController(pParent, "svx/ui/fontworkgallerydialog.ui", "FontworkGalleryDialog")
    , mnThemeId(0xffff)
    , mrSdrView(rSdrView)
    , mppSdrObject(nullptr)
    , mpDestModel(nullptr)
    , maCtlFavorites(m_xBuilder->weld_scrolled_window("ctlFavoriteswin"))
    , mxCtlFavorites(new weld::CustomWeld(*m_xBuilder, "ctlFavorites", maCtlFavorites))
    , mxOKButton(m_xBuilder->weld_button("ok"))
{
    Size aSize(maCtlFavorites.GetDrawingArea()->get_ref_device().LogicToPixel(Size(200, 200), MapMode(MapUnit::MapAppFont)));
    mxCtlFavorites->set_size_request(aSize.Width(), aSize.Height());

    maCtlFavorites.SetDoubleClickHdl( LINK( this, FontWorkGalleryDialog, DoubleClickFavoriteHdl ) );
    mxOKButton->connect_clicked(LINK(this, FontWorkGalleryDialog, ClickOKHdl));

    maCtlFavorites.SetColCount( nColCount );
    maCtlFavorites.SetLineCount( nLineCount );
    maCtlFavorites.SetExtraSpacing( 3 );

    initFavorites( GALLERY_THEME_FONTWORK );
    fillFavorites( GALLERY_THEME_FONTWORK );
}

FontWorkGalleryDialog::~FontWorkGalleryDialog()
{
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

            if (pVDev->GetDPIScaleFactor() > 1)
                aThumb.Scale(pVDev->GetDPIScaleFactor(), pVDev->GetDPIScaleFactor());

            const Size aSize(aThumb.GetSizePixel());

            pVDev->SetOutputSizePixel(aSize);

            static const sal_uInt32 nLen(8);
            static const Color aW(COL_WHITE);
            static const Color aG(0xef, 0xef, 0xef);

            pVDev->DrawCheckered(aNull, aSize, nLen, aW, aG);

            pVDev->DrawBitmapEx(aNull, aThumb);
            maFavoritesHorizontal.emplace_back(pVDev->GetBitmapEx(aNull, aSize));
        }
    }

    // release gallery theme
    GalleryExplorer::EndLocking(nThemeId);
}

void FontWorkGalleryDialog::fillFavorites(sal_uInt16 nThemeId)
{
    mnThemeId = nThemeId;

    Size aThumbSize(maCtlFavorites.GetOutputSizePixel());
    aThumbSize.setWidth( aThumbSize.Width() / nColCount );
    aThumbSize.setHeight( aThumbSize.Height() / nLineCount );
    aThumbSize.AdjustWidth( -12 );
    aThumbSize.AdjustHeight( -12 );

    std::vector< Bitmap * >::size_type nFavCount = maFavoritesHorizontal.size();

    // ValueSet favorites
    if( nFavCount > (nColCount * nLineCount) )
    {
        WinBits nWinBits = maCtlFavorites.GetStyle();
        nWinBits |= WB_VSCROLL;
        maCtlFavorites.SetStyle( nWinBits );
    }

    maCtlFavorites.Clear();

    for( std::vector<Bitmap *>::size_type nFavorite = 1; nFavorite <= nFavCount; nFavorite++ )
    {
        OUString aStr(SvxResId(RID_SVXFLOAT3D_FAVORITE));
        aStr += " ";
        aStr += OUString::number(nFavorite);
        Image aThumbImage( maFavoritesHorizontal[nFavorite-1] );
        maCtlFavorites.InsertItem( static_cast<sal_uInt16>(nFavorite), aThumbImage, aStr );
    }

    if (maCtlFavorites.GetItemCount())
        maCtlFavorites.SelectItem(1);
}

void FontWorkGalleryDialog::SetSdrObjectRef( SdrObject** ppSdrObject, SdrModel* pModel )
{
    mppSdrObject = ppSdrObject;
    mpDestModel = pModel;
}

void FontWorkGalleryDialog::insertSelectedFontwork()
{
    sal_uInt16 nItemId = maCtlFavorites.GetSelectedItemId();

    if( nItemId > 0 )
    {
        std::unique_ptr<FmFormModel> pModel(new FmFormModel());
        pModel->GetItemPool().FreezeIdRanges();

        if( GalleryExplorer::GetSdrObj( mnThemeId, nItemId-1, pModel.get() ) )
        {
            SdrPage* pPage = pModel->GetPage(0);
            if( pPage && pPage->GetObjCount() )
            {
                // tdf#116993 Calc uses a 'special' mode for this dialog in being the
                // only caller of ::SetSdrObjectRef. Only in that case mpDestModel seems
                // to be the correct target SdrModel.
                // If this is not used, the correct SdrModel seems to be the one from
                // the mrSdrView that is used to insert (InsertObjectAtView below) the
                // cloned SdrObject.
                const bool bUseSpecialCalcMode(nullptr != mppSdrObject && nullptr != mpDestModel);

                // center shape on current view
                OutputDevice* pOutDev(mrSdrView.GetFirstOutputDevice());

                if (pOutDev)
                {
                    // Clone directly to target SdrModel (may be different due to user/caller (!))
                    SdrObject* pNewObject(
                        pPage->GetObj(0)->CloneSdrObject(
                            bUseSpecialCalcMode ? *mpDestModel : mrSdrView.getSdrModelFromSdrView()));

                    // tdf#117629
                    // Since the 'old' ::CloneSdrObject also copies the SdrPage* the
                    // SdrObject::getUnoShape() *will* create the wrong UNO API object
                    // early. This IS one of the reasons I do change these things - this
                    // error does not happen with my next change I am working on already
                    // ARGH! For now, reset the SdrPage* to nullptr.
                    // What sense does it have to copy the SdrPage* of the original SdrObject ?!?
                    // TTTT: This also *might* be the hidden reason for the strange code at the
                    // end of SdrObject::SetPage that tries to delete the SvxShape under some
                    // circumstances...
                    // pNewObject->SetPage(nullptr);

                    tools::Rectangle aObjRect( pNewObject->GetLogicRect() );
                    tools::Rectangle aVisArea = pOutDev->PixelToLogic(tools::Rectangle(Point(0,0), pOutDev->GetOutputSizePixel()));
                    Point aPagePos = aVisArea.Center();
                    aPagePos.AdjustX( -(aObjRect.GetWidth() / 2) );
                    aPagePos.AdjustY( -(aObjRect.GetHeight() / 2) );
                    tools::Rectangle aNewObjectRectangle(aPagePos, aObjRect.GetSize());
                    pNewObject->SetLogicRect(aNewObjectRectangle);

                    if (bUseSpecialCalcMode)
                    {
                        *mppSdrObject = pNewObject;
                    }
                    else
                    {
                        SdrPageView* pPV(mrSdrView.GetSdrPageView());

                        if (nullptr != pPV)
                        {
                            mrSdrView.InsertObjectAtView( pNewObject, *pPV );
                        }
                        else
                        {
                            // tdf#116993 no target -> delete clone
                            SdrObject::Free(pNewObject);
                        }
                    }
                }
            }
        }
    }
}

IMPL_LINK_NOARG(FontWorkGalleryDialog, ClickOKHdl, weld::Button&, void)
{
    insertSelectedFontwork();
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(FontWorkGalleryDialog, DoubleClickFavoriteHdl, SvtValueSet*, void)
{
    insertSelectedFontwork();
    m_xDialog->response(RET_OK);
}

class FontworkAlignmentWindow : public ToolbarMenu
{
public:
    FontworkAlignmentWindow( svt::ToolboxController& rController, vcl::Window* pParentWindow );

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override;

private:
    svt::ToolboxController& mrController;

    DECL_LINK( SelectHdl, ToolbarMenu*, void );

    void    implSetAlignment( int nAlignmentMode, bool bEnabled );
};

static const OUStringLiteral gsFontworkAlignment(".uno:FontworkAlignment");

FontworkAlignmentWindow::FontworkAlignmentWindow(svt::ToolboxController& rController, vcl::Window* pParentWindow)
    : ToolbarMenu(rController.getFrameInterface(), pParentWindow, WB_STDPOPUP)
    , mrController(rController)
{
    SetSelectHdl( LINK( this, FontworkAlignmentWindow, SelectHdl ) );

    Image aImgAlgin1(StockImage::Yes, RID_SVXBMP_FONTWORK_ALIGN_LEFT);
    Image aImgAlgin2(StockImage::Yes, RID_SVXBMP_FONTWORK_ALIGN_CENTER);
    Image aImgAlgin3(StockImage::Yes, RID_SVXBMP_FONTWORK_ALIGN_RIGHT);
    Image aImgAlgin4(StockImage::Yes, RID_SVXBMP_FONTWORK_ALIGN_WORD);
    Image aImgAlgin5(StockImage::Yes, RID_SVXBMP_FONTWORK_ALIGN_STRETCH);

    appendEntry(0, SvxResId(RID_SVXSTR_ALIGN_LEFT), aImgAlgin1);
    appendEntry(1, SvxResId(RID_SVXSTR_ALIGN_CENTER), aImgAlgin2);
    appendEntry(2, SvxResId(RID_SVXSTR_ALIGN_RIGHT), aImgAlgin3);
    appendEntry(3, SvxResId(RID_SVXSTR_ALIGN_WORD), aImgAlgin4);
    appendEntry(4, SvxResId(RID_SVXSTR_ALIGN_STRETCH), aImgAlgin5);

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( gsFontworkAlignment );
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
    if( Event.FeatureURL.Main == gsFontworkAlignment )
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
        aArgs[0].Name = OUString(gsFontworkAlignment).copy(5);
        aArgs[0].Value <<= nAlignment;

        mrController.dispatchCommand( gsFontworkAlignment, aArgs );

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


OUString FontworkAlignmentControl::getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.FontworkAlignmentController" );
}


Sequence< OUString > FontworkAlignmentControl::getSupportedServiceNames()
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_FontworkAlignmentControl_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FontworkAlignmentControl(xContext));
}


class FontworkCharacterSpacingWindow : public ToolbarMenu
{
public:
    FontworkCharacterSpacingWindow( svt::ToolboxController& rController, vcl::Window* pParentWindow );

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override;
private:
    svt::ToolboxController& mrController;

    DECL_LINK( SelectHdl, ToolbarMenu*, void );

    void    implSetCharacterSpacing( sal_Int32 nCharacterSpacing, bool bEnabled );
    void    implSetKernCharacterPairs( bool bEnabled );

};
static const OUStringLiteral gsFontworkCharacterSpacing(".uno:FontworkCharacterSpacing");
static const OUStringLiteral gsFontworkKernCharacterPairs(".uno:FontworkKernCharacterPairs");

FontworkCharacterSpacingWindow::FontworkCharacterSpacingWindow(svt::ToolboxController& rController, vcl::Window* pParentWindow)
    : ToolbarMenu(rController.getFrameInterface(), pParentWindow, WB_STDPOPUP)
    , mrController(rController)
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

    AddStatusListener( gsFontworkCharacterSpacing );
    AddStatusListener( gsFontworkKernCharacterPairs );
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
    if( Event.FeatureURL.Main == gsFontworkCharacterSpacing )
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
    else if( Event.FeatureURL.Main == gsFontworkKernCharacterPairs )
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
        aArgs[0].Name = OUString(gsFontworkCharacterSpacing).copy(5);
        aArgs[0].Value <<= nCharacterSpacing;

        mrController.dispatchCommand( ".uno:FontworkCharacterSpacingDialog", aArgs );
    }
    else if ( nSelection == 6 ) // KernCharacterPairs
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString(gsFontworkKernCharacterPairs).copy(5);
        aArgs[0].Value <<= true;

        mrController.dispatchCommand( gsFontworkKernCharacterPairs, aArgs );

        implSetKernCharacterPairs( true );
    }
    else if( nSelection >= 0 )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString(gsFontworkCharacterSpacing).copy(5);
        aArgs[0].Value <<= nCharacterSpacing;

        mrController.dispatchCommand( gsFontworkCharacterSpacing,  aArgs );

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


OUString FontworkCharacterSpacingControl::getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.FontworkCharacterSpacingController" );
}


Sequence< OUString > FontworkCharacterSpacingControl::getSupportedServiceNames()
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_FontworkCharacterSpacingControl_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FontworkCharacterSpacingControl(xContext));
}

FontworkCharacterSpacingDialog::FontworkCharacterSpacingDialog(weld::Window* pParent, sal_Int32 nScale)
    : GenericDialogController(pParent, "svx/ui/fontworkspacingdialog.ui", "FontworkSpacingDialog")
    , m_xMtrScale(m_xBuilder->weld_metric_spin_button("entry", FieldUnit::PERCENT))
{
    m_xMtrScale->set_value(nScale, FieldUnit::PERCENT);
}

FontworkCharacterSpacingDialog::~FontworkCharacterSpacingDialog()
{
}

sal_Int32 FontworkCharacterSpacingDialog::getScale() const
{
    return static_cast<sal_Int32>(m_xMtrScale->get_value(FieldUnit::PERCENT));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
