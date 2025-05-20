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

#include <sal/log.hxx>
#include <vcl/svapp.hxx>

#include "ChartColorsPanel.hxx"
#include "ChartColorPaletteControl.hxx"

#include <ChartColorPaletteHelper.hxx>
#include <ChartController.hxx>
#include <ChartModel.hxx>
#include <DataSeries.hxx>

#include <com/sun/star/drawing/FillStyle.hpp>

#include <sfx2/weldutils.hxx>
#include <svtools/toolbarmenu.hxx>

#include <algorithm>

using namespace css;

namespace chart::sidebar
{
namespace
{
OUString getCID(const uno::Reference<frame::XModel>& xModel)
{
    const uno::Reference<frame::XController> xController(xModel->getCurrentController());
    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(xController, uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return {};

    uno::Any aAny = xSelectionSupplier->getSelection();
    if (!aAny.hasValue())
    {
        // if no selection, default to diagram wall so sidebar can show some editable properties
        if (auto* pController = dynamic_cast<ChartController*>(xController.get()))
        {
            pController->select(
                uno::Any(ObjectIdentifier::createClassifiedIdentifier(OBJECTTYPE_PAGE, u"")));
            xSelectionSupplier
                = uno::Reference<css::view::XSelectionSupplier>(xController, uno::UNO_QUERY);
            if (xSelectionSupplier.is())
                aAny = xSelectionSupplier->getSelection();
        }

        if (!aAny.hasValue())
            return {};
    }

    OUString aCID;
    aAny >>= aCID;
    return aCID;
}

ChartColorPaletteControl* getChartColorPaletteControl(const ToolbarUnoDispatcher& rToolBoxColor)
{
    const uno::Reference<frame::XToolbarController> xController
        = rToolBoxColor.GetControllerForCommand(sUnoChartColorPalette);
    const auto pToolBoxLineStyleControl
        = dynamic_cast<ChartColorPaletteControl*>(xController.get());
    return pToolBoxLineStyleControl;
}
} // end unnamed namespace

class ColorPaletteWrapper final : public IColorPaletteHandler
{
public:
    ColorPaletteWrapper(rtl::Reference<ChartModel> mxModel, ChartColorPaletteControl* pControl);

    void updateModel(const rtl::Reference<ChartModel>& xModel);
    void updateData() const;

    void select(ChartColorPaletteType eType, sal_uInt32 nIndex) override;
    void apply(const ChartColorPalette* pColorPalette) override;
    [[nodiscard]] std::shared_ptr<ChartColorPaletteHelper> getHelper() const override;
    [[nodiscard]] ChartColorPaletteType getType() const override;
    [[nodiscard]] sal_uInt32 getIndex() const override;

private:
    rtl::Reference<ChartModel> mxModel;
    ChartColorPaletteControl* mpControl;
};

ColorPaletteWrapper::ColorPaletteWrapper(rtl::Reference<ChartModel> xModel,
                                         ChartColorPaletteControl* pControl)
    : mxModel(std::move(xModel))
    , mpControl(pControl)
{
}

void ColorPaletteWrapper::updateModel(const rtl::Reference<ChartModel>& xModel)
{
    mxModel = xModel;
}

void ColorPaletteWrapper::updateData() const
{
    util::URL aUrl;
    aUrl.Complete = sUnoChartColorPalette;

    frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL = aUrl;
    aEvent.IsEnabled = true;

    if (mpControl)
        mpControl->statusChanged(aEvent);
}

void ColorPaletteWrapper::select(ChartColorPaletteType eType, const sal_uInt32 nIndex)
{
    mxModel->setColorPalette(eType, nIndex);
}

void ColorPaletteWrapper::apply(const ChartColorPalette* pColorPalette)
{
    if (pColorPalette)
        mxModel->applyColorPaletteToDataSeries(*pColorPalette);
}

std::shared_ptr<ChartColorPaletteHelper> ColorPaletteWrapper::getHelper() const
{
    const std::shared_ptr<model::Theme> pTheme = mxModel->getDocumentTheme();
    return std::make_shared<ChartColorPaletteHelper>(pTheme);
}

ChartColorPaletteType ColorPaletteWrapper::getType() const
{
    return mxModel->getColorPaletteType();
}

sal_uInt32 ColorPaletteWrapper::getIndex() const { return mxModel->getColorPaletteIndex(); }

const std::vector<ObjectType> ChartColorsPanel::maAcceptedTypes{
    OBJECTTYPE_PAGE,         OBJECTTYPE_LEGEND,        OBJECTTYPE_DIAGRAM,
    OBJECTTYPE_DIAGRAM_WALL, OBJECTTYPE_DIAGRAM_FLOOR, OBJECTTYPE_DATA_SERIES,
    OBJECTTYPE_DATA_POINT,
};

ChartColorsPanel::ChartColorsPanel(weld::Widget* pParent,
                                   const uno::Reference<frame::XFrame>& rxFrame,
                                   ChartController* pController)
    : PanelLayout(pParent, "ChartColorsPanel", "modules/schart/ui/sidebarcolors.ui")
    , mxModel(pController->getChartModel())
    , mxModifyListener(new ChartSidebarModifyListener(this))
    , mxSelectionListener(new ChartSidebarSelectionListener(this))
    , mbModelValid(true)
    , mxColorPaletteTB(m_xBuilder->weld_toolbar("colorpalettetype"))
    , mxColorPaletteDispatch(new ToolbarUnoDispatcher(*mxColorPaletteTB, *m_xBuilder, rxFrame))
{
    auto aAcceptedTypes(maAcceptedTypes);
    mxSelectionListener->setAcceptedTypes(std::move(aAcceptedTypes));
    Initialize();
}

ChartColorsPanel::~ChartColorsPanel()
{
    doUpdateModel(nullptr);
    mxColorPaletteDispatch.reset();
    mxColorPaletteTB.reset();
}

void ChartColorsPanel::Initialize()
{
    mxModel->addModifyListener(mxModifyListener);

    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(),
                                                                uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);

    ChartColorPaletteControl* pColorPaletteControl
        = getChartColorPaletteControl(*mxColorPaletteDispatch);
    assert(pColorPaletteControl);
    mxColorPaletteWrapper = std::make_shared<ColorPaletteWrapper>(mxModel, pColorPaletteControl);
    pColorPaletteControl->setColorPaletteHandler(mxColorPaletteWrapper);

    updateData();
}

void ChartColorsPanel::updateData()
{
    if (!mbModelValid)
        return;

    const OUString aCID = getCID(mxModel);
    if (aCID.isEmpty())
        return;
    const ObjectType eType = ObjectIdentifier::getObjectType(aCID);

    if (std::find(maAcceptedTypes.begin(), maAcceptedTypes.end(), eType) == maAcceptedTypes.end())
        return;

    // if fill style is not solid clear palette selection
    if (eType == OBJECTTYPE_DATA_SERIES || eType == OBJECTTYPE_DATA_POINT)
    {
        const uno::Reference<beans::XPropertySet> xPropSet
            = ObjectIdentifier::getObjectPropertySet(aCID, mxModel);
        if (!xPropSet.is())
            return;

        uno::Reference<beans::XPropertySetInfo> xInfo(xPropSet->getPropertySetInfo());
        if (!xInfo.is())
            return;

        SolarMutexGuard aGuard;
        if (xInfo->hasPropertyByName("FillStyle"))
        {
            drawing::FillStyle eFillStyle = drawing::FillStyle_SOLID;
            xPropSet->getPropertyValue("FillStyle") >>= eFillStyle;
            if (eFillStyle != drawing::FillStyle_SOLID)
            {
                mxModel->clearColorPalette();
            }
        }
    }

    mxColorPaletteWrapper->updateData();
}

std::unique_ptr<PanelLayout> ChartColorsPanel::Create(weld::Widget* pParent,
                                                      const uno::Reference<frame::XFrame>& rxFrame,
                                                      ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ChartColorsPanel::Create",
                                             nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartColorsPanel::Create", nullptr,
                                             1);

    return std::make_unique<ChartColorsPanel>(pParent, rxFrame, pController);
}

void ChartColorsPanel::DataChanged(const DataChangedEvent& rEvent)
{
    PanelLayout::DataChanged(rEvent);
    updateData();
}

void ChartColorsPanel::HandleContextChange(const vcl::EnumContext&) { updateData(); }

void ChartColorsPanel::NotifyItemUpdate(sal_uInt16 /*nSID*/, SfxItemState /*eState*/,
                                        const SfxPoolItem* /*pState*/)
{
}

void ChartColorsPanel::modelInvalid() { mbModelValid = false; }

void ChartColorsPanel::doUpdateModel(const rtl::Reference<ChartModel>& xModel)
{
    if (mbModelValid)
    {
        mxModel->removeModifyListener(mxModifyListener);

        const uno::Reference<view::XSelectionSupplier> oldSelectionSupplier(
            mxModel->getCurrentController(), uno::UNO_QUERY);
        if (oldSelectionSupplier.is())
        {
            oldSelectionSupplier->removeSelectionChangeListener(mxSelectionListener);
        }
    }

    mxModel = xModel;
    mbModelValid = mxModel.is();

    if (!mbModelValid)
        return;

    mxColorPaletteWrapper->updateModel(mxModel);

    mxModel->addModifyListener(mxModifyListener);

    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(),
                                                                uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);
}

void ChartColorsPanel::updateModel(const uno::Reference<frame::XModel> xModel)
{
    const auto pModel = dynamic_cast<ChartModel*>(xModel.get());
    assert(!xModel || pModel);
    doUpdateModel(pModel);
}

void ChartColorsPanel::selectionChanged(const bool bCorrectType)
{
    if (bCorrectType)
        updateData();
}

} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
