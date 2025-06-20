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

#include <com/sun/star/drawing/FillStyle.hpp>

#include <memory>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>

#include <ChartColorPaletteHelper.hxx>
#include "ChartColorPaletteControl.hxx"

#include <vcl/event.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>

using namespace css;

namespace chart::sidebar
{
constexpr tools::Long BORDER = ChartColorPaletteLayout::ItemBorder;
constexpr tools::Long SIZE = ChartColorPaletteLayout::ItemSize;
constexpr tools::Long constElementNumber = ChartColorPaletteSize;

ChartColorPaletteControl::ChartColorPaletteControl(
    const uno::Reference<uno::XComponentContext>& rContext)
    : PopupWindowController(rContext, nullptr, OUString())
    , meColorPaletteType(ChartColorPaletteType::Unknown)
    , mnColorPaletteIndex(0)
{
}

ChartColorPaletteControl::~ChartColorPaletteControl() = default;

void ChartColorPaletteControl::initialize(const uno::Sequence<uno::Any>& rArguments)
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

void ChartColorPaletteControl::execute(sal_Int16 /*nKeyModifier*/)
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

void ChartColorPaletteControl::statusChanged(const frame::FeatureStateEvent& rEvent)
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

void ChartColorPaletteControl::updateStatus(bool bForce)
{
    if (!mpHandler)
        return;

    if (bForce || meColorPaletteType != getColorPaletteType()
        || mnColorPaletteIndex != getColorPaletteIndex())
    {
        meColorPaletteType = getColorPaletteType();
        mnColorPaletteIndex = getColorPaletteIndex();

        ToolBox* pToolBox = nullptr;
        ToolBoxItemId nId;
        if (!getToolboxId(nId, &pToolBox) && !m_pToolbar)
            return;

        auto pDev = VclPtr<VirtualDevice>::Create();
        renderSelectedColorPalette(pDev);
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

void ChartColorPaletteControl::createDiagramSnapshot() const
{
    if (mpHandler)
        mpHandler->createDiagramSnapshot();
}

void ChartColorPaletteControl::restoreOriginalDiagram() const
{
    if (mpHandler)
        mpHandler->restoreOriginalDiagram();
}

void ChartColorPaletteControl::renderSelectedColorPalette(const VclPtr<VirtualDevice>& pDev) const
{
    if (!pDev)
        return;

    const auto pHelper = getColorPaletteHelper();
    if (!pHelper)
        return;

    static constexpr Point aPosition(0, 0);
    static constexpr Size aSize
        = { BORDER * 6 + SIZE * constElementNumber / 2, BORDER * 5 + SIZE * 2 };
    static constexpr tools::Rectangle aDrawArea(aPosition, aSize);

    pDev->SetOutputSizePixel(aSize, /*bErase*/ true, /*bAlphaMaskTransparent*/ true);

    if (getColorPaletteType() == ChartColorPaletteType::Unknown)
    {
        ChartColorPaletteHelper::renderNoPalette(pDev, aDrawArea);
        return;
    }

    const ChartColorPalette aColorPalette
        = pHelper->getColorPalette(getColorPaletteType(), getColorPaletteIndex());
    ChartColorPaletteHelper::renderColorPalette(pDev, aDrawArea, aColorPalette, true);
}

void ChartColorPaletteControl::setColorPaletteHandler(
    std::shared_ptr<IColorPaletteHandler> rColorPaletteHandler)
{
    if (!mpHandler)
    {
        mpHandler = rColorPaletteHandler;
        updateStatus(true);
    }
}

std::shared_ptr<ChartColorPaletteHelper> ChartColorPaletteControl::getColorPaletteHelper() const
{
    if (mpHandler)
        return mpHandler->getHelper();
    return nullptr;
}

ChartColorPaletteType ChartColorPaletteControl::getColorPaletteType() const
{
    if (mpHandler)
        return mpHandler->getType();
    return ChartColorPaletteType::Unknown;
}

sal_uInt32 ChartColorPaletteControl::getColorPaletteIndex() const
{
    if (mpHandler)
        return mpHandler->getIndex();
    return 0;
}

void ChartColorPaletteControl::dispatchColorPaletteCommand(const ChartColorPaletteType eType,
                                                           const sal_uInt32 nIndex) const
{
    if (mpHandler)
        mpHandler->select(eType, nIndex);
}

void ChartColorPaletteControl::applyColorPalette(const ChartColorPalette* pColorPalette) const
{
    if (mpHandler)
        mpHandler->apply(pColorPalette);
}

std::unique_ptr<WeldToolbarPopup> ChartColorPaletteControl::weldPopupWindow()
{
    return std::make_unique<ChartColorPalettePopup>(this, m_pToolbar);
}

VclPtr<vcl::Window> ChartColorPaletteControl::createVclPopupWindow(vcl::Window* pParent)
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(
        getFrameInterface(), pParent,
        std::make_unique<ChartColorPalettePopup>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

OUString ChartColorPaletteControl::getImplementationName()
{
    return "com.sun.star.comp.chart2.ChartColorPaletteControl";
}

uno::Sequence<OUString> ChartColorPaletteControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_chart2_ChartColorPaletteControl_get_implementation(
    uno::XComponentContext* rContext, uno::Sequence<uno::Any> const&)
{
    return cppu::acquire(new ChartColorPaletteControl(rContext));
}

ChartColorPalettePopup::ChartColorPalettePopup(ChartColorPaletteControl* pControl,
                                               weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent,
                       "modules/schart/ui/chartcolorpalettepopup.ui", "ColorPaletteWindow")
    , mxControl(pControl)
    , mxColorfulValueSet(new ChartColorPalettes)
    , mxColorfulValueSetWin(
          new weld::CustomWeld(*m_xBuilder, "colorful_palettes", *mxColorfulValueSet))
    , mxMonoValueSet(new ChartColorPalettes)
    , mxMonoValueSetWin(
          new weld::CustomWeld(*m_xBuilder, "monochromatic_palettes", *mxMonoValueSet))
    , meHighlightedItemType(mxControl->getColorPaletteType())
    , mnHighlightedItemId(mxControl->getColorPaletteIndex())
    , mbItemSelected(false)
{
    mxColorfulValueSet->SetColCount(2);
    mxColorfulValueSet->SetLineCount(2);
    mxColorfulValueSet->SetColor(Application::GetSettings().GetStyleSettings().GetFaceColor());

    mxMonoValueSet->SetColCount(2);
    mxMonoValueSet->SetLineCount(3);
    mxMonoValueSet->SetColor(Application::GetSettings().GetStyleSettings().GetFaceColor());

    initColorPalettes();

    mxColorfulValueSet->SetOptimalSize();
    mxColorfulValueSet->SetSelectHdl(LINK(this, ChartColorPalettePopup, SelectColorfulValueSetHdl));
    mxColorfulValueSet->setMouseMoveHdl(LINK(this, ChartColorPalettePopup, ColorfulMouseMoveHdl));

    mxMonoValueSet->SetOptimalSize();
    mxMonoValueSet->SetSelectHdl(LINK(this, ChartColorPalettePopup, SelectMonoValueSetHdl));
    mxMonoValueSet->setMouseMoveHdl(LINK(this, ChartColorPalettePopup, MonoMouseMoveHdl));

    selectItem(mxControl->getColorPaletteType(), mxControl->getColorPaletteIndex() + 1);

    mxControl->createDiagramSnapshot();
}

ChartColorPalettePopup::~ChartColorPalettePopup()
{
    if (!mbItemSelected)
        mxControl->restoreOriginalDiagram();
}

void ChartColorPalettePopup::selectItem(const ChartColorPaletteType eType,
                                        const sal_uInt32 nIndex) const
{
    switch (eType)
    {
        default:
        case ChartColorPaletteType::Unknown:
            mxColorfulValueSet->SetNoSelection();
            mxMonoValueSet->SetNoSelection();
            break;
        case ChartColorPaletteType::Colorful:
            mxColorfulValueSet->SelectItem(nIndex);
            break;
        case ChartColorPaletteType::Monochromatic:
            mxMonoValueSet->SelectItem(nIndex);
            break;
    }
}

void ChartColorPalettePopup::initColorPalettes() const
{
    const auto pColorPaletteHelper = mxControl->getColorPaletteHelper();
    if (!pColorPaletteHelper)
        return;
    // colorful palettes
    for (size_t i = 0; i < ChartColorPaletteHelper::ColorfulPaletteSize; ++i)
        mxColorfulValueSet->insert(
            pColorPaletteHelper->getColorPalette(ChartColorPaletteType::Colorful, i));
    // monotonic palettes
    for (size_t i = 0; i < ChartColorPaletteHelper::MonotonicPaletteSize; ++i)
        mxMonoValueSet->insert(
            pColorPaletteHelper->getColorPalette(ChartColorPaletteType::Monochromatic, i));
}

void ChartColorPalettePopup::GrabFocus()
{
    if (mxMonoValueSet->IsNoSelection())
        mxColorfulValueSet->GrabFocus();
    else
        mxMonoValueSet->GrabFocus();
}

IMPL_LINK_NOARG(ChartColorPalettePopup, SelectColorfulValueSetHdl, ValueSet*, void)
{
    sal_uInt32 nIndex = SelectValueSetHdl(mxColorfulValueSet);
    if (nIndex != static_cast<sal_uInt32>(-1))
    {
        mxControl->dispatchColorPaletteCommand(ChartColorPaletteType::Colorful, nIndex);
        mxMonoValueSet->SetNoSelection();
        mxControl->updateStatus();
    }
    mxControl->EndPopupMode();
}

IMPL_LINK_NOARG(ChartColorPalettePopup, SelectMonoValueSetHdl, ValueSet*, void)
{
    sal_uInt32 nIndex = SelectValueSetHdl(mxMonoValueSet);
    if (nIndex != static_cast<sal_uInt32>(-1))
    {
        mxControl->dispatchColorPaletteCommand(ChartColorPaletteType::Monochromatic, nIndex);
        mxColorfulValueSet->SetNoSelection();
        mxControl->updateStatus();
    }
    mxControl->EndPopupMode();
}

sal_uInt32
ChartColorPalettePopup::SelectValueSetHdl(const std::unique_ptr<ChartColorPalettes>& xValueSet)
{
    const sal_uInt32 nItemId = xValueSet->GetSelectedItemId();

    if (!nItemId)
        return sal_uInt32(-1);

    const sal_uInt32 nIndex = nItemId - 1;

    if (const ChartColorPalette* pPalette = xValueSet->getPalette(nIndex))
    {
        mxControl->applyColorPalette(pPalette);
        mbItemSelected = true;
        return nIndex;
    }
    return sal_uInt32(-1);
}

IMPL_LINK_NOARG(ChartColorPalettePopup, ColorfulMouseMoveHdl, const MouseEvent&, void)
{
    MouseMoveHdl(mxColorfulValueSet, ChartColorPaletteType::Colorful);
}

IMPL_LINK_NOARG(ChartColorPalettePopup, MonoMouseMoveHdl, const MouseEvent&, void)
{
    MouseMoveHdl(mxMonoValueSet, ChartColorPaletteType::Monochromatic);
}

void ChartColorPalettePopup::MouseMoveHdl(const std::unique_ptr<ChartColorPalettes>& xValueSet,
                                          const ChartColorPaletteType eHlItemType)
{
    const sal_uInt16 nHlId = xValueSet->GetHighlightedItemId();
    if (eHlItemType == meHighlightedItemType && nHlId == mnHighlightedItemId)
        return;

    if (nHlId > 0)
    {
        if (const ChartColorPalette* pPalette = xValueSet->getPalette(nHlId - 1))
        {
            mxControl->applyColorPalette(pPalette);
        }
    }
    else
    {
        mxControl->restoreOriginalDiagram();
    }

    meHighlightedItemType = eHlItemType;
    mnHighlightedItemId = nHlId;
}

} // end namespace chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
