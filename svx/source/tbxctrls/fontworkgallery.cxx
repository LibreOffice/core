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
#include <vcl/virdev.hxx>

#include <svl/itempool.hxx>

#include <svtools/toolbarmenu.hxx>
#include <svtools/popupwindowcontroller.hxx>

#include <svx/fmmodel.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>

#include <svx/gallery.hxx>
#include <svx/fontworkgallery.hxx>

#include <algorithm>
#include <memory>

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

    auto nFavCount = maFavoritesHorizontal.size();

    // ValueSet favorites
    if( nFavCount > (nColCount * nLineCount) )
    {
        WinBits nWinBits = maCtlFavorites.GetStyle();
        nWinBits |= WB_VSCROLL;
        maCtlFavorites.SetStyle( nWinBits );
    }

    maCtlFavorites.Clear();

    for( size_t nFavorite = 1; nFavorite <= nFavCount; nFavorite++ )
    {
        OUString aStr = SvxResId(RID_SVXFLOAT3D_FAVORITE) + " " + OUString::number(nFavorite);
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

    if( nItemId <= 0 )
        return;

    std::unique_ptr<FmFormModel> pModel(new FmFormModel());
    pModel->GetItemPool().FreezeIdRanges();

    if( !GalleryExplorer::GetSdrObj( mnThemeId, nItemId-1, pModel.get() ) )
        return;

    SdrPage* pPage = pModel->GetPage(0);
    if( !(pPage && pPage->GetObjCount()) )
        return;

    // tdf#116993 Calc uses a 'special' mode for this dialog in being the
    // only caller of ::SetSdrObjectRef. Only in that case mpDestModel seems
    // to be the correct target SdrModel.
    // If this is not used, the correct SdrModel seems to be the one from
    // the mrSdrView that is used to insert (InsertObjectAtView below) the
    // cloned SdrObject.
    const bool bUseSpecialCalcMode(nullptr != mppSdrObject && nullptr != mpDestModel);

    // center shape on current view
    OutputDevice* pOutDev(mrSdrView.GetFirstOutputDevice());

    if (!pOutDev)
        return;

    // Clone directly to target SdrModel (may be different due to user/caller (!))
    SdrObject* pNewObject(
        pPage->GetObj(0)->CloneSdrObject(
            bUseSpecialCalcMode ? *mpDestModel : mrSdrView.getSdrModelFromSdrView()));

    pNewObject->MakeNameUnique();

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

IMPL_LINK_NOARG(FontWorkGalleryDialog, ClickOKHdl, weld::Button&, void)
{
    insertSelectedFontwork();
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(FontWorkGalleryDialog, DoubleClickFavoriteHdl, ValueSet*, void)
{
    insertSelectedFontwork();
    m_xDialog->response(RET_OK);
}

namespace {

class FontworkAlignmentWindow final : public WeldToolbarPopup
{
public:
    FontworkAlignmentWindow(svt::PopupWindowController* pControl, weld::Widget* pParentWindow);
    virtual void GrabFocus() override
    {
        mxLeft->grab_focus();
    }
    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override;

private:
    rtl::Reference<svt::PopupWindowController> mxControl;
    std::unique_ptr<weld::RadioButton> mxLeft;
    std::unique_ptr<weld::RadioButton> mxCenter;
    std::unique_ptr<weld::RadioButton> mxRight;
    std::unique_ptr<weld::RadioButton> mxWord;
    std::unique_ptr<weld::RadioButton> mxStretch;
    bool mbSettingValue;

    DECL_LINK( SelectHdl, weld::ToggleButton&, void );

    void    implSetAlignment( int nAlignmentMode, bool bEnabled );
};

}

constexpr OUStringLiteral gsFontworkAlignment(u".uno:FontworkAlignment");

FontworkAlignmentWindow::FontworkAlignmentWindow(svt::PopupWindowController* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "svx/ui/fontworkalignmentcontrol.ui", "FontworkAlignmentControl")
    , mxControl(pControl)
    , mxLeft(m_xBuilder->weld_radio_button("left"))
    , mxCenter(m_xBuilder->weld_radio_button("center"))
    , mxRight(m_xBuilder->weld_radio_button("right"))
    , mxWord(m_xBuilder->weld_radio_button("word"))
    , mxStretch(m_xBuilder->weld_radio_button("stretch"))
    , mbSettingValue(false)
{
    mxLeft->connect_toggled(LINK(this, FontworkAlignmentWindow, SelectHdl));
    mxCenter->connect_toggled(LINK(this, FontworkAlignmentWindow, SelectHdl));
    mxRight->connect_toggled(LINK(this, FontworkAlignmentWindow, SelectHdl));
    mxWord->connect_toggled(LINK(this, FontworkAlignmentWindow, SelectHdl));
    mxStretch->connect_toggled(LINK(this, FontworkAlignmentWindow, SelectHdl));

    AddStatusListener( gsFontworkAlignment );
}

void FontworkAlignmentWindow::implSetAlignment( int nSurface, bool bEnabled )
{
    bool bSettingValue = mbSettingValue;
    mbSettingValue = true;
    mxLeft->set_active(nSurface == 0 && bEnabled);
    mxLeft->set_sensitive(bEnabled);
    mxCenter->set_active(nSurface == 1 && bEnabled);
    mxCenter->set_sensitive(bEnabled);
    mxRight->set_active(nSurface == 2 && bEnabled);
    mxRight->set_sensitive(bEnabled);
    mxWord->set_active(nSurface == 3 && bEnabled);
    mxWord->set_sensitive(bEnabled);
    mxStretch->set_active(nSurface == 4 && bEnabled);
    mxStretch->set_sensitive(bEnabled);
    mbSettingValue = bSettingValue;
}

void FontworkAlignmentWindow::statusChanged( const css::frame::FeatureStateEvent& Event )
{
    if( Event.FeatureURL.Main != gsFontworkAlignment )
        return;

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

IMPL_LINK(FontworkAlignmentWindow, SelectHdl, weld::ToggleButton&, rButton, void)
{
    if (mbSettingValue || !rButton.get_active())
        return;

    sal_Int32 nAlignment;
    if (mxLeft->get_active())
        nAlignment = 0;
    else if (mxCenter->get_active())
        nAlignment = 1;
    else if (mxRight->get_active())
        nAlignment = 2;
    else if (mxWord->get_active())
        nAlignment = 3;
    else
        nAlignment = 4;

    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name = OUString(gsFontworkAlignment).copy(5);
    aArgs[0].Value <<= nAlignment;

    mxControl->dispatchCommand( gsFontworkAlignment, aArgs );

    implSetAlignment( nAlignment, true );

    mxControl->EndPopupMode();
}

namespace {

class FontworkAlignmentControl : public svt::PopupWindowController
{
public:
    explicit FontworkAlignmentControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

}

FontworkAlignmentControl::FontworkAlignmentControl( const Reference< XComponentContext >& rxContext )
: svt::PopupWindowController( rxContext, Reference< css::frame::XFrame >(), ".uno:FontworkAlignment" )
{
}

std::unique_ptr<WeldToolbarPopup> FontworkAlignmentControl::weldPopupWindow()
{
    return std::make_unique<FontworkAlignmentWindow>(this, m_pToolbar);
}

VclPtr<vcl::Window> FontworkAlignmentControl::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<FontworkAlignmentWindow>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
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
    return "com.sun.star.comp.svx.FontworkAlignmentController";
}


Sequence< OUString > FontworkAlignmentControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_FontworkAlignmentControl_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FontworkAlignmentControl(xContext));
}

namespace {

class FontworkCharacterSpacingWindow final : public WeldToolbarPopup
{
public:
    FontworkCharacterSpacingWindow(svt::PopupWindowController* pControl, weld::Widget* pParentWindow);
    virtual void GrabFocus() override;

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override;
private:
    rtl::Reference<svt::PopupWindowController> mxControl;
    std::unique_ptr<weld::RadioButton> mxVeryTight;
    std::unique_ptr<weld::RadioButton> mxTight;
    std::unique_ptr<weld::RadioButton> mxNormal;
    std::unique_ptr<weld::RadioButton> mxLoose;
    std::unique_ptr<weld::RadioButton> mxVeryLoose;
    std::unique_ptr<weld::RadioButton> mxCustom;
    std::unique_ptr<weld::CheckButton> mxKernPairs;
    bool mbSettingValue;

    DECL_LINK( SelectHdl, weld::ToggleButton&, void );
    DECL_LINK( ClickHdl, weld::Button&, void );

    void    implSetCharacterSpacing( sal_Int32 nCharacterSpacing, bool bEnabled );
    void    implSetKernCharacterPairs(bool bKernOnOff, bool bEnabled);
};

}

constexpr OUStringLiteral gsFontworkCharacterSpacing(u".uno:FontworkCharacterSpacing");
constexpr OUStringLiteral gsFontworkKernCharacterPairs(u".uno:FontworkKernCharacterPairs");

FontworkCharacterSpacingWindow::FontworkCharacterSpacingWindow(svt::PopupWindowController* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "svx/ui/fontworkcharacterspacingcontrol.ui", "FontworkCharacterSpacingControl")
    , mxControl(pControl)
    , mxVeryTight(m_xBuilder->weld_radio_button("verytight"))
    , mxTight(m_xBuilder->weld_radio_button("tight"))
    , mxNormal(m_xBuilder->weld_radio_button("normal"))
    , mxLoose(m_xBuilder->weld_radio_button("loose"))
    , mxVeryLoose(m_xBuilder->weld_radio_button("veryloose"))
    , mxCustom(m_xBuilder->weld_radio_button("custom"))
    , mxKernPairs(m_xBuilder->weld_check_button("kernpairs"))
    , mbSettingValue(false)
{
    mxVeryTight->connect_toggled(LINK(this, FontworkCharacterSpacingWindow, SelectHdl));
    mxTight->connect_toggled(LINK(this, FontworkCharacterSpacingWindow, SelectHdl));
    mxNormal->connect_toggled(LINK(this, FontworkCharacterSpacingWindow, SelectHdl));
    mxLoose->connect_toggled(LINK(this, FontworkCharacterSpacingWindow, SelectHdl));
    mxVeryLoose->connect_toggled(LINK(this, FontworkCharacterSpacingWindow, SelectHdl));
    mxCustom->connect_clicked(LINK(this, FontworkCharacterSpacingWindow, ClickHdl));

    mxKernPairs->connect_toggled(LINK(this, FontworkCharacterSpacingWindow, SelectHdl));

    AddStatusListener( gsFontworkCharacterSpacing );
    AddStatusListener( gsFontworkKernCharacterPairs );

    // See TODO in svx/source/toolbars/fontworkbar.cxx for SID_FONTWORK_KERN_CHARACTER_PAIRS,
    // the kernpairs setting is ignored, so hide the widget entirely
    mxKernPairs->hide();
}

void FontworkCharacterSpacingWindow::GrabFocus()
{
    mxVeryTight->grab_focus();
}

void FontworkCharacterSpacingWindow::implSetCharacterSpacing( sal_Int32 nCharacterSpacing, bool bEnabled )
{
    bool bSettingValue = mbSettingValue;
    mbSettingValue = true;

    mxVeryTight->set_sensitive(bEnabled);
    mxTight->set_sensitive(bEnabled);
    mxNormal->set_sensitive(bEnabled);
    mxLoose->set_sensitive(bEnabled);
    mxVeryLoose->set_sensitive(bEnabled);
    mxCustom->set_sensitive(bEnabled);

    mxVeryTight->set_active(false);
    mxTight->set_active(false);
    mxNormal->set_active(false);
    mxLoose->set_active(false);
    mxVeryLoose->set_active(false);
    mxCustom->set_active(true);

    switch(nCharacterSpacing)
    {
        case 80:
            mxVeryTight->set_active(true);
            break;
        case 90:
            mxTight->set_active(true);
            break;
        case 100:
            mxNormal->set_active(true);
            break;
        case 120:
            mxLoose->set_active(true);
            break;
        case 150:
            mxVeryLoose->set_active(true);
            break;
    }

    mbSettingValue = bSettingValue;
}

void FontworkCharacterSpacingWindow::implSetKernCharacterPairs(bool bKernOnOff, bool bEnabled)
{
    mxKernPairs->set_sensitive(bEnabled);
    mxKernPairs->set_active(bKernOnOff);
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
            implSetKernCharacterPairs(false, false);
        }
        else
        {
            bool bValue = false;
            if( Event.State >>= bValue )
                implSetKernCharacterPairs(bValue, true);
        }
    }
}

IMPL_LINK_NOARG(FontworkCharacterSpacingWindow, ClickHdl, weld::Button&, void)
{
    SelectHdl(*mxCustom);
}

IMPL_LINK(FontworkCharacterSpacingWindow, SelectHdl, weld::ToggleButton&, rButton, void)
{
    if (mbSettingValue || !rButton.get_active())
        return;

    if (&rButton == mxCustom.get())
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString(gsFontworkCharacterSpacing).copy(5);
        aArgs[0].Value <<= sal_Int32(100);

        rtl::Reference<svt::PopupWindowController> xControl(mxControl);
        xControl->EndPopupMode();
        xControl->dispatchCommand(".uno:FontworkCharacterSpacingDialog", aArgs);
    }
    else if (&rButton == mxKernPairs.get())
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString(gsFontworkKernCharacterPairs).copy(5);
        bool bKernOnOff = mxKernPairs->get_active();
        aArgs[0].Value <<= bKernOnOff;

        mxControl->dispatchCommand( gsFontworkKernCharacterPairs, aArgs );

        implSetKernCharacterPairs(bKernOnOff, true);
    }
    else
    {
        sal_Int32 nCharacterSpacing;
        if (&rButton == mxVeryTight.get())
            nCharacterSpacing = 80;
        else if (&rButton == mxTight.get())
            nCharacterSpacing = 90;
        else if (&rButton == mxLoose.get())
            nCharacterSpacing = 120;
        else if (&rButton == mxVeryLoose.get())
            nCharacterSpacing = 150;
        else
            nCharacterSpacing = 100;

        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString(gsFontworkCharacterSpacing).copy(5);
        aArgs[0].Value <<= nCharacterSpacing;

        mxControl->dispatchCommand( gsFontworkCharacterSpacing,  aArgs );

        implSetCharacterSpacing( nCharacterSpacing, true );
    }

    mxControl->EndPopupMode();
}

namespace {

class FontworkCharacterSpacingControl : public svt::PopupWindowController
{
public:
    explicit FontworkCharacterSpacingControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

}

FontworkCharacterSpacingControl::FontworkCharacterSpacingControl( const Reference< XComponentContext >& rxContext )
: svt::PopupWindowController( rxContext, Reference< css::frame::XFrame >(), ".uno:FontworkCharacterSpacingFloater" )
{
}

std::unique_ptr<WeldToolbarPopup> FontworkCharacterSpacingControl::weldPopupWindow()
{
    return std::make_unique<FontworkCharacterSpacingWindow>(this, m_pToolbar);
}

VclPtr<vcl::Window> FontworkCharacterSpacingControl::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<FontworkCharacterSpacingWindow>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
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
    return "com.sun.star.comp.svx.FontworkCharacterSpacingController";
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
