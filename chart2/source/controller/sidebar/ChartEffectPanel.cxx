/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ChartEffectPanel.hxx"

#include <ChartController.hxx>
#include <ChartModel.hxx>
#include <ViewElementListProvider.hxx>

#include <sfx2/weldutils.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/xcolit.hxx>
#include <vcl/svapp.hxx>

namespace chart::sidebar
{
namespace
{
OUString getCID(const rtl::Reference<::chart::ChartModel>& xModel)
{
    css::uno::Reference<css::frame::XController> xController(xModel->getCurrentController());
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController,
                                                                          css::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return OUString();

    css::uno::Any aAny = xSelectionSupplier->getSelection();
    if (!aAny.hasValue())
    {
        // if no selection, default to diagram wall so sidebar can show some editable properties
        ChartController* pController = dynamic_cast<ChartController*>(xController.get());
        if (pController)
        {
            pController->select(
                css::uno::Any(ObjectIdentifier::createClassifiedIdentifier(OBJECTTYPE_PAGE, u"")));
            xSelectionSupplier = css::uno::Reference<css::view::XSelectionSupplier>(
                xController, css::uno::UNO_QUERY);
            if (xSelectionSupplier.is())
                aAny = xSelectionSupplier->getSelection();
        }

        if (!aAny.hasValue())
            return OUString();
    }

    OUString aCID;
    aAny >>= aCID;

    return aCID;
}

css::uno::Reference<css::beans::XPropertySet>
getPropSet(const rtl::Reference<::chart::ChartModel>& xModel)
{
    OUString aCID = getCID(xModel);
    css::uno::Reference<css::beans::XPropertySet> xPropSet
        = ObjectIdentifier::getObjectPropertySet(aCID, xModel);

    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if (eType == OBJECTTYPE_DIAGRAM)
    {
        css::uno::Reference<css::chart2::XDiagram> xDiagram(xPropSet, css::uno::UNO_QUERY);
        if (!xDiagram.is())
            return xPropSet;

        xPropSet.set(xDiagram->getWall());
    }

    return xPropSet;
}

class PreventUpdate
{
public:
    explicit PreventUpdate(bool& bUpdate)
        : mbUpdate(bUpdate)
    {
        mbUpdate = false;
    }

    ~PreventUpdate() { mbUpdate = true; }

private:
    bool& mbUpdate;
};
}

std::unique_ptr<PanelLayout> ChartEffectPanel::Create(weld::Widget* pParent,
                                                      ChartController* pController)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no parent Window given to EffectPropertyPanel::Create"_ustr, nullptr, 0);

    return std::make_unique<ChartEffectPanel>(pParent, pController);
}

ChartEffectPanel::ChartEffectPanel(weld::Widget* pParent, ChartController* pController)
    : svx::sidebar::EffectPropertyPanelBase(pParent)
    , mxModel(pController->getChartModel())
    , mxListener(new ChartSidebarModifyListener(this))
    , mxSelectionListener(new ChartSidebarSelectionListener(this))
    , mbUpdate(true)
    , mbModelValid(true)
{
    std::vector<ObjectType> aAcceptedTypes{ OBJECTTYPE_PAGE,        OBJECTTYPE_DIAGRAM,
                                            OBJECTTYPE_DATA_SERIES, OBJECTTYPE_DATA_POINT,
                                            OBJECTTYPE_TITLE,       OBJECTTYPE_LEGEND };
    mxSelectionListener->setAcceptedTypes(std::move(aAcceptedTypes));
    Initialize();
}

ChartEffectPanel::~ChartEffectPanel() { doUpdateModel(nullptr); }

void ChartEffectPanel::Initialize()
{
    mxModel->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(
        mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);

    updateData();
}

void ChartEffectPanel::setGlowColor(const XColorItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    const css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue(u"GlowEffectColor"_ustr, css::uno::Any(rItem.GetColorValue()));
}

void ChartEffectPanel::setGlowRadius(const SdrMetricItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue(u"GlowEffectRadius"_ustr, css::uno::Any(rItem.GetValue()));
}

void ChartEffectPanel::setGlowTransparency(const SdrPercentItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue(u"GlowEffectTransparency"_ustr, css::uno::Any(rItem.GetValue()));
}

void ChartEffectPanel::setSoftEdgeRadius(const SdrMetricItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    const css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue(u"SoftEdgeRadius"_ustr, css::uno::Any(rItem.GetValue()));
}

void ChartEffectPanel::updateData()
{
    if (!mbUpdate || !mbModelValid)
        return;

    const css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    const css::uno::Reference<css::beans::XPropertySetInfo> xInfo(xPropSet->getPropertySetInfo());
    if (!xInfo.is())
        return;

    SolarMutexGuard aGuard;
    if (xInfo->hasPropertyByName(u"GlowEffectRadius"_ustr))
    {
        sal_Int32 nRadius = 0;
        xPropSet->getPropertyValue(u"GlowEffectRadius"_ustr) >>= nRadius;
        const SdrMetricItem aRadiusItem(SDRATTR_GLOW_RADIUS, nRadius);
        updateGlowRadius(true, &aRadiusItem);
    }

    if (xInfo->hasPropertyByName(u"GlowEffectColor"_ustr))
    {
        sal_uInt32 nColor = 0;
        xPropSet->getPropertyValue(u"GlowEffectColor"_ustr) >>= nColor;
        const XColorItem aColorItem(SDRATTR_GLOW_COLOR, Color(ColorTransparency, nColor));
        updateGlowColor(true, &aColorItem);
    }

    if (xInfo->hasPropertyByName(u"GlowEffectTransparency"_ustr))
    {
        sal_Int16 nTransparency = 0;
        xPropSet->getPropertyValue(u"GlowEffectTransparency"_ustr) >>= nTransparency;
        const SdrPercentItem aTransparencyItem(SDRATTR_GLOW_TRANSPARENCY, nTransparency);
        updateGlowTransparency(true, &aTransparencyItem);
    }
}

void ChartEffectPanel::modelInvalid() { mbModelValid = false; }

void ChartEffectPanel::selectionChanged(bool bCorrectType)
{
    if (bCorrectType)
        updateData();
}

void ChartEffectPanel::doUpdateModel(const rtl::Reference<::chart::ChartModel>& xModel)
{
    if (mbModelValid)
    {
        mxModel->removeModifyListener(mxListener);

        css::uno::Reference<css::view::XSelectionSupplier> oldSelectionSupplier(
            mxModel->getCurrentController(), css::uno::UNO_QUERY);
        if (oldSelectionSupplier.is())
        {
            oldSelectionSupplier->removeSelectionChangeListener(mxSelectionListener);
        }
    }

    mxModel = xModel;
    mbModelValid = mxModel.is();

    if (!mbModelValid)
        return;

    mxModel->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(
        mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);
}

void ChartEffectPanel::updateModel(css::uno::Reference<css::frame::XModel> xModel)
{
    ::chart::ChartModel* pModel = dynamic_cast<::chart::ChartModel*>(xModel.get());
    assert(!xModel || pModel);
    doUpdateModel(pModel);
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
