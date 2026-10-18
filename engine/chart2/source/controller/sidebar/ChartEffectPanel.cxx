/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ChartEffectPanel.hxx"

#include <ChartController.hxx>
#include <ChartModel.hxx>
#include <ViewElementListProvider.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <sfx2/weldutils.hxx>
#include <svx/svddef.hxx>
#include <svx/tbcontrl.hxx>
#include <vcl/svapp.hxx>

namespace chart::sidebar
{
namespace
{
OUString getCID(const rtl::Reference<::chart::ChartModel>& xModel)
{
    cpo::uno::Reference<css::frame::XController> xController(xModel->getCurrentController());
    cpo::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController,
                                                                          cpo::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return OUString();

    cpo::uno::Any aAny = xSelectionSupplier->getSelection();
    if (!aAny.hasValue())
    {
        // if no selection, default to diagram wall so sidebar can show some editable properties
        ChartController* pController = dynamic_cast<ChartController*>(xController.get());
        if (pController)
        {
            pController->select(
                cpo::uno::Any(ObjectIdentifier::createClassifiedIdentifier(OBJECTTYPE_PAGE, u"")));
            xSelectionSupplier = cpo::uno::Reference<css::view::XSelectionSupplier>(
                xController, cpo::uno::UNO_QUERY);
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

cpo::uno::Reference<css::beans::XPropertySet>
getPropSet(const rtl::Reference<::chart::ChartModel>& xModel)
{
    OUString aCID = getCID(xModel);
    cpo::uno::Reference<css::beans::XPropertySet> xPropSet
        = ObjectIdentifier::getObjectPropertySet(aCID, xModel);

    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if (eType == OBJECTTYPE_DIAGRAM)
    {
        cpo::uno::Reference<css::chart2::XDiagram> xDiagram(xPropSet, cpo::uno::UNO_QUERY);
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

    cpo::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(
        mxModel->getCurrentController(), cpo::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);

    updateData();
}

void ChartEffectPanel::setGlow(const SdrGlowEffectItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    const cpo::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    // The chart model has the glow as three properties, with the radius in 1/100 mm.
    xPropSet->setPropertyValue(u"GlowEffectRadius"_ustr,
                               cpo::uno::Any(rItem.getEffect().maRadius.as_hmm<sal_Int32>()));
    xPropSet->setPropertyValue(u"GlowEffectColor"_ustr, cpo::uno::Any(rItem.getColor()));
    xPropSet->setPropertyValue(u"GlowEffectTransparency"_ustr,
                               cpo::uno::Any(rItem.getTransparency()));
}

void ChartEffectPanel::setSoftEdge(const SdrSoftEdgeEffectItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    const cpo::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    // The chart model has the radius in 1/100 mm.
    xPropSet->setPropertyValue(u"SoftEdgeRadius"_ustr,
                               cpo::uno::Any(rItem.getEffect().maRadius.as_hmm<sal_Int32>()));
}

void ChartEffectPanel::updateData()
{
    if (!mbUpdate || !mbModelValid)
        return;

    const cpo::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    const cpo::uno::Reference<css::beans::XPropertySetInfo> xInfo(xPropSet->getPropertySetInfo());
    if (!xInfo.is())
        return;

    SolarMutexGuard aGuard;
    if (xInfo->hasPropertyByName(u"GlowEffectRadius"_ustr))
    {
        sal_Int32 nRadius = 0;
        sal_uInt32 nColor = 0;
        sal_Int16 nTransparency = 0;
        xPropSet->getPropertyValue(u"GlowEffectRadius"_ustr) >>= nRadius;
        xPropSet->getPropertyValue(u"GlowEffectColor"_ustr) >>= nColor;
        xPropSet->getPropertyValue(u"GlowEffectTransparency"_ustr) >>= nTransparency;

        SdrGlowEffectItem aGlow(SDRATTR_GLOW);
        aGlow.setRadius(gfx::Length::hmm(nRadius));
        aGlow.setColor(Color(ColorTransparency, nColor));
        aGlow.setTransparency(nTransparency);
        updateGlow(true, &aGlow);
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

        cpo::uno::Reference<css::view::XSelectionSupplier> oldSelectionSupplier(
            mxModel->getCurrentController(), cpo::uno::UNO_QUERY);
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

    cpo::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(
        mxModel->getCurrentController(), cpo::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);
}

void ChartEffectPanel::updateModel(cpo::uno::Reference<css::frame::XModel> xModel)
{
    ::chart::ChartModel* pModel = dynamic_cast<::chart::ChartModel*>(xModel.get());
    assert(!xModel || pModel);
    doUpdateModel(pModel);
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
