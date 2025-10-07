/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <com/sun/star/drawing/FillStyle.hpp>

#include <memory>
#include <utility>
#include <o3tl/enumrange.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>

#include <ChartGradientPaletteHelper.hxx>
#include "ChartGradientPaletteControl.hxx"

#include <vcl/event.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>

using namespace css;

namespace chart::sidebar
{
constexpr tools::Long BORDER = ChartGradientPaletteLayout::ItemBorder;
constexpr tools::Long SIZE = ChartGradientPaletteLayout::ItemSize;

ChartGradientPaletteControl::ChartGradientPaletteControl(
    const uno::Reference<uno::XComponentContext>& rContext)
    : PopupWindowController(rContext, nullptr, OUString())
    , meGradientVariation(ChartGradientVariation::Unknown)
    , mnGradientType(ChartGradientType::Invalid)
{
}

ChartGradientPaletteControl::~ChartGradientPaletteControl() = default;

void ChartGradientPaletteControl::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    svt::PopupWindowController::initialize(rArguments);

    if (m_pToolbar)
    {
        mxPopoverContainer = std::make_unique<ToolbarPopupContainer>(m_pToolbar);
        m_pToolbar->set_item_popover(m_aCommandURL, mxPopoverContainer->getTopLevel());
        m_pToolbar->set_item_sensitive(m_aCommandURL, true);
    }

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (getToolboxId(nId, &pToolBox))
    {
        pToolBox->SetItemBits(nId, pToolBox->GetItemBits(nId) | ToolBoxItemBits::DROPDOWNONLY);
        pToolBox->EnableItem(nId, true);
    }
}

void ChartGradientPaletteControl::execute(sal_Int16 /*nKeyModifier*/)
{
    if (m_pToolbar)
    {
        // Toggle the popup also when toolbutton is activated
        m_pToolbar->set_menu_item_active(m_aCommandURL,
                                         !m_pToolbar->get_menu_item_active(m_aCommandURL));
    }
    else
    {
        // Open the popup also when Enter key is pressed.
        createPopupWindow();
    }
}

void ChartGradientPaletteControl::statusChanged(const frame::FeatureStateEvent& rEvent)
{
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (!getToolboxId(nId, &pToolBox) && !m_pToolbar)
        return;

    if (rEvent.FeatureURL.Complete == m_aCommandURL)
    {
        updateStatus();
    }
}

void ChartGradientPaletteControl::updateStatus(bool bForce)
{
    if (!mpHandler)
        return;

    if (bForce || meGradientVariation != getGradientVariation()
        || mnGradientType != getGradientType())
    {
        meGradientVariation = getGradientVariation();
        mnGradientType = getGradientType();

        ToolBox* pToolBox = nullptr;
        ToolBoxItemId nId;
        if (!getToolboxId(nId, &pToolBox) && !m_pToolbar)
            return;

        auto pDev = VclPtr<VirtualDevice>::Create();
        renderSelectedGradientPalette(pDev);
        auto aSelItemImg(pDev->GetBitmapEx(Point(), pDev->GetOutputSizePixel()));
        if (m_pToolbar)
        {
            m_pToolbar->set_item_image(m_aCommandURL, Graphic(aSelItemImg).GetXGraphic());
        }
        else
        {
            pToolBox->SetItemImage(nId, Image(aSelItemImg));
        }
    }
}

void ChartGradientPaletteControl::createDiagramSnapshot() const
{
    if (mpHandler)
        mpHandler->createDiagramSnapshot();
}

void ChartGradientPaletteControl::restoreOriginalDiagram() const
{
    if (mpHandler)
        mpHandler->restoreOriginalDiagram();
}

void ChartGradientPaletteControl::renderSelectedGradientPalette(
    const VclPtr<VirtualDevice>& pDev) const
{
    if (!pDev)
        return;

    const auto pHelper = getGradientPaletteHelper();
    if (!pHelper)
        return;

    static constexpr Point aPosition(0, 0);
    static constexpr Size aSize = { 2 * BORDER + SIZE, 2 * BORDER + SIZE };
    static constexpr tools::Rectangle aDrawArea(aPosition, aSize);

    pDev->SetOutputSizePixel(aSize, /*bErase*/ true, /*bAlphaMaskTransparent*/ true);

    if (getGradientVariation() == ChartGradientVariation::Unknown)
    {
        ChartGradientPaletteHelper::renderNoGradient(pDev, aDrawArea);
        return;
    }

    const basegfx::BGradient aGradientItem
        = pHelper->getGradientSample(getGradientVariation(), getGradientType());
    ChartGradientPaletteHelper::renderGradientItem(pDev, aDrawArea, aGradientItem, true);
}

void ChartGradientPaletteControl::setGradientPaletteHandler(
    std::shared_ptr<IGradientPaletteHandler> rGradientPaletteHandler)
{
    if (!mpHandler)
    {
        mpHandler = rGradientPaletteHandler;
        updateStatus(true);
    }
}

std::shared_ptr<ChartGradientPaletteHelper>
ChartGradientPaletteControl::getGradientPaletteHelper() const
{
    if (mpHandler)
        return mpHandler->getHelper();
    return nullptr;
}

ChartGradientVariation ChartGradientPaletteControl::getGradientVariation() const
{
    if (mpHandler)
        return mpHandler->getVariation();
    return ChartGradientVariation::Unknown;
}

ChartGradientType ChartGradientPaletteControl::getGradientType() const
{
    if (mpHandler)
        return mpHandler->getType();
    return ChartGradientType::Invalid;
}

void ChartGradientPaletteControl::dispatchGradientPaletteCommand(
    const ChartGradientVariation eVariant, const ChartGradientType eType) const
{
    if (mpHandler)
        mpHandler->select(eVariant, eType);
}

void ChartGradientPaletteControl::applyGradientPalette(const ChartGradientVariation eVariation,
                                                       const ChartGradientType eType,
                                                       const bool bIsPreview) const
{
    if (!mpHandler)
        return;

    mpHandler->setPreview(bIsPreview);

    if (const auto pGradientPaletteHelper = getGradientPaletteHelper())
        mpHandler->apply(pGradientPaletteHelper->getGradientPalette(eVariation, eType));

    mpHandler->setPreview(false);
}

std::unique_ptr<WeldToolbarPopup> ChartGradientPaletteControl::weldPopupWindow()
{
    return std::make_unique<ChartGradientPalettePopup>(this, m_pToolbar);
}

VclPtr<vcl::Window> ChartGradientPaletteControl::createVclPopupWindow(vcl::Window* pParent)
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(
        getFrameInterface(), pParent,
        std::make_unique<ChartGradientPalettePopup>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

OUString ChartGradientPaletteControl::getImplementationName()
{
    return "com.sun.star.comp.chart2.ChartGradientPaletteControl";
}

uno::Sequence<OUString> ChartGradientPaletteControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_chart2_ChartGradientPaletteControl_get_implementation(
    uno::XComponentContext* rContext, uno::Sequence<uno::Any> const&)
{
    return cppu::acquire(new ChartGradientPaletteControl(rContext));
}

ChartGradientPalettePopup::ChartGradientPalettePopup(ChartGradientPaletteControl* pControl,
                                                     weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent,
                       "modules/schart/ui/chartgradientpalettepopup.ui", "GradientPaletteWindow")
    , mxControl(pControl)
    , mxLightPalettes(new ChartGradientPalettes(*m_xBuilder, "light_palettes", "lightwin"))
    , mxDarkPalettes(new ChartGradientPalettes(*m_xBuilder, "dark_palettes", "darkwin"))
    , meHighlightedItemVariation(mxControl->getGradientVariation())
    , mnHighlightedItemType(mxControl->getGradientType())
    , mbItemSelected(false)
{
    mxLightPalettes->SetSelectHdl(LINK(this, ChartGradientPalettePopup, SelectLightPaletteHdl));
    mxLightPalettes->setMouseMoveHdl(
        LINK(this, ChartGradientPalettePopup, LightPaletteMouseMoveHdl));

    mxDarkPalettes->SetSelectHdl(LINK(this, ChartGradientPalettePopup, SelectDarkPaletteHdl));
    mxDarkPalettes->setMouseMoveHdl(LINK(this, ChartGradientPalettePopup, DarkPaletteMouseMoveHdl));

    initGradientPalettes();

    selectItem(mxControl->getGradientVariation(), mxControl->getGradientType());

    mxControl->createDiagramSnapshot();
}

ChartGradientPalettePopup::~ChartGradientPalettePopup()
{
    if (!mbItemSelected)
        mxControl->restoreOriginalDiagram();
}

void ChartGradientPalettePopup::selectItem(const ChartGradientVariation eVariation,
                                           const ChartGradientType eType) const
{
    const sal_uInt16 nIndex = static_cast<sal_uInt16>(eType) + 1;

    switch (eVariation)
    {
        default:
        case ChartGradientVariation::Unknown:
            mxLightPalettes->SetNoSelection();
            mxDarkPalettes->SetNoSelection();
            break;
        case ChartGradientVariation::LightVariation:
            mxLightPalettes->SelectItem(nIndex);
            break;
        case ChartGradientVariation::DarkVariation:
            mxDarkPalettes->SelectItem(nIndex);
            break;
    }
}

void ChartGradientPalettePopup::initGradientPalettes() const
{
    const auto pGradientPaletteHelper = mxControl->getGradientPaletteHelper();
    if (!pGradientPaletteHelper)
        return;

    for (auto type : o3tl::enumrange<ChartGradientType>())
    {
        mxLightPalettes->insert(pGradientPaletteHelper->getGradientSample(
            ChartGradientVariation::LightVariation, type));
        mxDarkPalettes->insert(
            pGradientPaletteHelper->getGradientSample(ChartGradientVariation::DarkVariation, type));
    }
    mxLightPalettes->Fill();
    mxDarkPalettes->Fill();
}

void ChartGradientPalettePopup::GrabFocus()
{
    if (mxDarkPalettes->IsNoSelection())
        mxLightPalettes->GrabFocus();
    else
        mxDarkPalettes->GrabFocus();
}

IMPL_LINK_NOARG(ChartGradientPalettePopup, SelectLightPaletteHdl, weld::IconView&, bool)
{
    sal_uInt32 nIndex = GetSelectedItem(mxLightPalettes);
    if (nIndex != static_cast<sal_uInt32>(-1))
    {
        const auto eType = static_cast<ChartGradientType>(nIndex);
        mxControl->applyGradientPalette(ChartGradientVariation::LightVariation, eType);
        mxControl->dispatchGradientPaletteCommand(ChartGradientVariation::LightVariation, eType);
        mxDarkPalettes->SetNoSelection();
        mxControl->updateStatus();
    }
    mxControl->EndPopupMode();
    return true;
}

IMPL_LINK_NOARG(ChartGradientPalettePopup, SelectDarkPaletteHdl, weld::IconView&, bool)
{
    sal_uInt32 nIndex = GetSelectedItem(mxDarkPalettes);
    if (nIndex != static_cast<sal_uInt32>(-1))
    {
        const auto eType = static_cast<ChartGradientType>(nIndex);
        mxControl->applyGradientPalette(ChartGradientVariation::DarkVariation, eType);
        mxControl->dispatchGradientPaletteCommand(ChartGradientVariation::DarkVariation, eType);
        mxLightPalettes->SetNoSelection();
        mxControl->updateStatus();
    }
    mxControl->EndPopupMode();
    return true;
}

sal_uInt32
ChartGradientPalettePopup::GetSelectedItem(const std::unique_ptr<ChartGradientPalettes>& xPalettes)
{
    const sal_uInt32 nItemId = xPalettes->GetSelectedItemId();
    if (!nItemId)
        return static_cast<sal_uInt32>(-1);

    const sal_uInt32 nIndex = nItemId - 1;

    if (const basegfx::BGradient* pPalette = xPalettes->getPalette(nIndex))
    {
        mbItemSelected = true;
        return nIndex;
    }
    return static_cast<sal_uInt32>(-1);
}

IMPL_LINK_NOARG(ChartGradientPalettePopup, LightPaletteMouseMoveHdl, const MouseEvent&, bool)
{
    MouseMoveHdl(mxLightPalettes, ChartGradientVariation::LightVariation);
    return true;
}

IMPL_LINK_NOARG(ChartGradientPalettePopup, DarkPaletteMouseMoveHdl, const MouseEvent&, bool)
{
    MouseMoveHdl(mxDarkPalettes, ChartGradientVariation::DarkVariation);
    return true;
}

void ChartGradientPalettePopup::MouseMoveHdl(
    const std::unique_ptr<ChartGradientPalettes>& xPalettes,
    const ChartGradientVariation eHlItemVariation)
{
    const sal_uInt16 nHlId = xPalettes->GetHighlightedItemId();
    const ChartGradientType eHlType
        = nHlId > 0 ? static_cast<ChartGradientType>(nHlId - 1) : ChartGradientType::Invalid;
    if (eHlItemVariation == meHighlightedItemVariation && eHlType == mnHighlightedItemType)
        return;

    if (nHlId > 0)
    {
        if (const basegfx::BGradient* pPalette = xPalettes->getPalette(nHlId - 1))
        {
            mxControl->applyGradientPalette(eHlItemVariation, eHlType, true);
        }
    }
    else
    {
        mxControl->restoreOriginalDiagram();
    }

    meHighlightedItemVariation = eHlItemVariation;
    mnHighlightedItemType = eHlType;
}

} // end namespace chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
