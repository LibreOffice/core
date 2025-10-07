/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/log.hxx>
#include <vcl/svapp.hxx>

#include "ChartGradientsPanel.hxx"
#include "ChartGradientPaletteControl.hxx"

#include <ChartGradientPaletteHelper.hxx>
#include <ChartController.hxx>
#include <ChartModel.hxx>
#include <DataSeries.hxx>
#include <Diagram.hxx>

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

ChartGradientPaletteControl*
getChartGradientPaletteControl(const ToolbarUnoDispatcher& rToolBoxGradient)
{
    const uno::Reference<frame::XToolbarController> xController
        = rToolBoxGradient.GetControllerForCommand(sUnoChartGradientPalette);
    const auto pToolBoxLineStyleControl
        = dynamic_cast<ChartGradientPaletteControl*>(xController.get());
    return pToolBoxLineStyleControl;
}
} // end unnamed namespace

class GradientPaletteWrapper final : public IGradientPaletteHandler
{
public:
    GradientPaletteWrapper(rtl::Reference<ChartModel> mxModel,
                           ChartGradientPaletteControl* pControl);

    void updateModel(const rtl::Reference<ChartModel>& xModel);
    void updateData() const;

    void createDiagramSnapshot() override;
    void restoreOriginalDiagram() override;

    void select(ChartGradientVariation eVariant, ChartGradientType eType) override;
    void apply(const ChartGradientPalette& rGradientPalette) override;
    void setPreview(bool bFlag) override;
    [[nodiscard]] std::shared_ptr<ChartGradientPaletteHelper> getHelper() const override;
    [[nodiscard]] ChartGradientVariation getVariation() const override;
    [[nodiscard]] ChartGradientType getType() const override;

private:
    rtl::Reference<ChartModel> mxModel;
    ChartGradientPaletteControl* mpControl;
    rtl::Reference<Diagram> mxDiagramSnapshot;
    bool mbIsPreview;
};

GradientPaletteWrapper::GradientPaletteWrapper(rtl::Reference<ChartModel> xModel,
                                               ChartGradientPaletteControl* pControl)
    : mxModel(std::move(xModel))
    , mpControl(pControl)
{
}

void GradientPaletteWrapper::updateModel(const rtl::Reference<ChartModel>& xModel)
{
    mxModel = xModel;
}

void GradientPaletteWrapper::updateData() const
{
    util::URL aUrl;
    aUrl.Complete = sUnoChartGradientPalette;

    frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL = aUrl;
    aEvent.IsEnabled = true;

    if (mpControl)
        mpControl->statusChanged(aEvent);
}

void GradientPaletteWrapper::createDiagramSnapshot()
{
    const rtl::Reference<Diagram> xDiagram = mxModel->getFirstChartDiagram();
    mxDiagramSnapshot = new ::chart::Diagram(*xDiagram);
}

void GradientPaletteWrapper::restoreOriginalDiagram()
{
    if (mxDiagramSnapshot)
    {
        const rtl::Reference<Diagram> xDiagram = new ::chart::Diagram(*mxDiagramSnapshot);
        // setDiagram didn't make a copy internally, so we need to pass a copy or
        // the diagram snapshot would be modified on preview
        mxModel->setFirstDiagram(xDiagram);
    }
}

void GradientPaletteWrapper::select(ChartGradientVariation eVariant, ChartGradientType eType)
{
    mxModel->clearColorPalette();
    mxModel->setGradientPalette(eVariant, eType);
}

void GradientPaletteWrapper::apply(const ChartGradientPalette& rGradientPalette)
{
    mxModel->applyGradientPaletteToDataSeries(rGradientPalette);
}

void GradientPaletteWrapper::setPreview(bool bFlag) { mbIsPreview = bFlag; }

std::shared_ptr<ChartGradientPaletteHelper> GradientPaletteWrapper::getHelper() const
{
    return std::make_shared<ChartGradientPaletteHelper>(
        mxModel->getDataSeriesColorsForGradient(mbIsPreview));
}

ChartGradientVariation GradientPaletteWrapper::getVariation() const
{
    return mxModel->getGradientPaletteVariation();
}

ChartGradientType GradientPaletteWrapper::getType() const
{
    return mxModel->getGradientPaletteType();
}

const std::vector<ObjectType> ChartGradientsPanel::maAcceptedTypes{
    OBJECTTYPE_PAGE,         OBJECTTYPE_LEGEND,        OBJECTTYPE_DIAGRAM,
    OBJECTTYPE_DIAGRAM_WALL, OBJECTTYPE_DIAGRAM_FLOOR, OBJECTTYPE_DATA_SERIES,
    OBJECTTYPE_DATA_POINT,
};

ChartGradientsPanel::ChartGradientsPanel(weld::Widget* pParent,
                                         const uno::Reference<frame::XFrame>& rxFrame,
                                         ChartController* pController)
    : PanelLayout(pParent, "ChartGradientsPanel", "modules/schart/ui/sidebargradients.ui")
    , mxModel(pController->getChartModel())
    , mxModifyListener(new ChartSidebarModifyListener(this))
    , mxSelectionListener(new ChartSidebarSelectionListener(this))
    , mbModelValid(true)
    , mxGradientPaletteTB(m_xBuilder->weld_toolbar("gradientpalettetype"))
    , mxGradientPaletteDispatch(
          new ToolbarUnoDispatcher(*mxGradientPaletteTB, *m_xBuilder, rxFrame))
{
    auto aAcceptedTypes(maAcceptedTypes);
    mxSelectionListener->setAcceptedTypes(std::move(aAcceptedTypes));
    Initialize();
}

ChartGradientsPanel::~ChartGradientsPanel()
{
    doUpdateModel(nullptr);
    mxGradientPaletteDispatch.reset();
    mxGradientPaletteTB.reset();
}

void ChartGradientsPanel::Initialize()
{
    mxModel->addModifyListener(mxModifyListener);

    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(),
                                                                uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);

    ChartGradientPaletteControl* pGradientPaletteControl
        = getChartGradientPaletteControl(*mxGradientPaletteDispatch);
    assert(pGradientPaletteControl);
    mxGradientPaletteWrapper
        = std::make_shared<GradientPaletteWrapper>(mxModel, pGradientPaletteControl);
    pGradientPaletteControl->setGradientPaletteHandler(mxGradientPaletteWrapper);

    updateData();
}

void ChartGradientsPanel::updateData()
{
    if (!mbModelValid)
        return;

    const OUString aCID = getCID(mxModel);
    if (aCID.isEmpty())
        return;
    const ObjectType eType = ObjectIdentifier::getObjectType(aCID);

    if (std::find(maAcceptedTypes.begin(), maAcceptedTypes.end(), eType) == maAcceptedTypes.end())
        return;

    // if fill style is not a gradient clear palette selection
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
            if (eFillStyle != drawing::FillStyle_GRADIENT)
            {
                mxModel->clearGradientPalette();
            }
        }
    }

    mxGradientPaletteWrapper->updateData();
}

std::unique_ptr<PanelLayout>
ChartGradientsPanel::Create(weld::Widget* pParent, const uno::Reference<frame::XFrame>& rxFrame,
                            ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            "no parent Window given to ChartGradientsPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartGradientsPanel::Create",
                                             nullptr, 1);

    return std::make_unique<ChartGradientsPanel>(pParent, rxFrame, pController);
}

void ChartGradientsPanel::DataChanged(const DataChangedEvent& rEvent)
{
    PanelLayout::DataChanged(rEvent);
    updateData();
}

void ChartGradientsPanel::HandleContextChange(const vcl::EnumContext&) { updateData(); }

void ChartGradientsPanel::NotifyItemUpdate(sal_uInt16 /*nSID*/, SfxItemState /*eState*/,
                                           const SfxPoolItem* /*pState*/)
{
}

void ChartGradientsPanel::modelInvalid() { mbModelValid = false; }

void ChartGradientsPanel::doUpdateModel(const rtl::Reference<ChartModel>& xModel)
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

    mxGradientPaletteWrapper->updateModel(mxModel);

    mxModel->addModifyListener(mxModifyListener);

    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(),
                                                                uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);
}

void ChartGradientsPanel::updateModel(const uno::Reference<frame::XModel> xModel)
{
    const auto pModel = dynamic_cast<ChartModel*>(xModel.get());
    assert(!xModel || pModel);
    doUpdateModel(pModel);
}

void ChartGradientsPanel::selectionChanged(const bool bCorrectType)
{
    if (bCorrectType)
        updateData();
}

} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
