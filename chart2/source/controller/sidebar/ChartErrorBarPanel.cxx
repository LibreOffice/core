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

#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "ChartErrorBarPanel.hxx"
#include <ChartController.hxx>
#include <ChartModel.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>


using namespace css;
using namespace css::uno;

namespace chart::sidebar {

namespace {

enum class ErrorBarDirection
{
    POSITIVE,
    NEGATIVE
};

css::uno::Reference<css::beans::XPropertySet> getErrorBarPropSet(
        const rtl::Reference<::chart::ChartModel>& xModel, std::u16string_view rCID)
{
    return ObjectIdentifier::getObjectPropertySet(rCID, xModel);
}

bool showPositiveError(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return false;

    css::uno::Any aAny = xPropSet->getPropertyValue(u"ShowPositiveError"_ustr);

    if (!aAny.hasValue())
        return false;

    bool bShow = false;
    aAny >>= bShow;
    return bShow;
}

bool showNegativeError(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return false;

    css::uno::Any aAny = xPropSet->getPropertyValue(u"ShowNegativeError"_ustr);

    if (!aAny.hasValue())
        return false;

    bool bShow = false;
    aAny >>= bShow;
    return bShow;
}

void setShowPositiveError(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID, bool bShow)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue(u"ShowPositiveError"_ustr, css::uno::Any(bShow));
}

void setShowNegativeError(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID, bool bShow)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue(u"ShowNegativeError"_ustr, css::uno::Any(bShow));
}

struct ErrorBarTypeMap
{
    sal_Int32 nPos;
    sal_Int32 nApi;
};

ErrorBarTypeMap const aErrorBarType[] = {
    { 0, css::chart::ErrorBarStyle::ABSOLUTE },
    { 1, css::chart::ErrorBarStyle::RELATIVE },
    { 2, css::chart::ErrorBarStyle::FROM_DATA },
    { 3, css::chart::ErrorBarStyle::STANDARD_DEVIATION },
    { 4, css::chart::ErrorBarStyle::STANDARD_ERROR },
    { 5, css::chart::ErrorBarStyle::VARIANCE},
    { 6, css::chart::ErrorBarStyle::ERROR_MARGIN },
};

sal_Int32 getTypePos(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return 0;

    css::uno::Any aAny = xPropSet->getPropertyValue(u"ErrorBarStyle"_ustr);

    if (!aAny.hasValue())
        return 0;

    sal_Int32 nApi = 0;
    aAny >>= nApi;

    for (ErrorBarTypeMap const & i : aErrorBarType)
    {
        if (i.nApi == nApi)
            return i.nPos;
    }

    return 0;
}

void setTypePos(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID, sal_Int32 nPos)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return;

    sal_Int32 nApi = 0;
    for (ErrorBarTypeMap const & i : aErrorBarType)
    {
        if (i.nPos == nPos)
            nApi = i.nApi;
    }

    xPropSet->setPropertyValue(u"ErrorBarStyle"_ustr, css::uno::Any(nApi));
}

double getValue(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID, ErrorBarDirection eDir)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return 0;

    OUString aName = u"PositiveError"_ustr;
    if (eDir == ErrorBarDirection::NEGATIVE)
        aName = "NegativeError";

    css::uno::Any aAny = xPropSet->getPropertyValue(aName);

    if (!aAny.hasValue())
        return 0;

    double nVal = 0;
    aAny >>= nVal;

    return nVal;
}

void setValue(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID, double nVal, ErrorBarDirection eDir)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return;

    OUString aName = u"PositiveError"_ustr;
    if (eDir == ErrorBarDirection::NEGATIVE)
        aName = "NegativeError";

    xPropSet->setPropertyValue(aName, css::uno::Any(nVal));
}

OUString getCID(const rtl::Reference<::chart::ChartModel>& xModel)
{
    css::uno::Reference<css::frame::XController> xController(xModel->getCurrentController());
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController, css::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return OUString();

    uno::Any aAny = xSelectionSupplier->getSelection();
    assert(aAny.hasValue());
    OUString aCID;
    aAny >>= aCID;
#if defined DBG_UTIL && !defined NDEBUG
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if (eType != OBJECTTYPE_DATA_ERRORS_X &&
         eType != OBJECTTYPE_DATA_ERRORS_Y &&
         eType != OBJECTTYPE_DATA_ERRORS_Z)
        SAL_WARN("chart2","Selected item is not an error bar");

#endif

    return aCID;
}

}

ChartErrorBarPanel::ChartErrorBarPanel(weld::Widget* pParent, ChartController* pController)
    : PanelLayout(pParent, u"ChartErrorBarPanel"_ustr, u"modules/schart/ui/sidebarerrorbar.ui"_ustr)
    , mxRBPosAndNeg(m_xBuilder->weld_radio_button(u"radiobutton_positive_negative"_ustr))
    , mxRBPos(m_xBuilder->weld_radio_button(u"radiobutton_positive"_ustr))
    , mxRBNeg(m_xBuilder->weld_radio_button(u"radiobutton_negative"_ustr))
    , mxLBType(m_xBuilder->weld_combo_box(u"comboboxtext_type"_ustr))
    , mxMFPos(m_xBuilder->weld_spin_button(u"spinbutton_pos"_ustr))
    , mxMFNeg(m_xBuilder->weld_spin_button(u"spinbutton_neg"_ustr))
    , mxModel(pController->getChartModel())
    , mxListener(new ChartSidebarModifyListener(this))
    , mbModelValid(true)
{
    Initialize();
}

ChartErrorBarPanel::~ChartErrorBarPanel()
{
    doUpdateModel(nullptr);

    mxRBPosAndNeg.reset();
    mxRBPos.reset();
    mxRBNeg.reset();

    mxLBType.reset();

    mxMFPos.reset();
    mxMFNeg.reset();
}

void ChartErrorBarPanel::Initialize()
{
    mxModel->addModifyListener(mxListener);
    mxRBNeg->set_active(false);
    mxRBPos->set_active(false);
    mxRBPosAndNeg->set_active(false);

    updateData();

    Link<weld::Toggleable&,void> aLink = LINK(this, ChartErrorBarPanel, RadioBtnHdl);
    mxRBPosAndNeg->connect_toggled(aLink);
    mxRBPos->connect_toggled(aLink);
    mxRBNeg->connect_toggled(aLink);

    mxLBType->connect_changed(LINK(this, ChartErrorBarPanel, ListBoxHdl));

    Link<weld::SpinButton&,void> aLink2 = LINK(this, ChartErrorBarPanel, NumericFieldHdl);
    mxMFPos->connect_value_changed(aLink2);
    mxMFNeg->connect_value_changed(aLink2);
}

void ChartErrorBarPanel::updateData()
{
    if (!mbModelValid)
        return;

    OUString aCID = getCID(mxModel);
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if (eType != OBJECTTYPE_DATA_ERRORS_X &&
         eType != OBJECTTYPE_DATA_ERRORS_Y &&
         eType != OBJECTTYPE_DATA_ERRORS_Z)
        return;

    bool bPos = showPositiveError(mxModel, aCID);
    bool bNeg = showNegativeError(mxModel, aCID);

    SolarMutexGuard aGuard;

    if (bPos && bNeg)
        mxRBPosAndNeg->set_active(true);
    else if (bPos)
        mxRBPos->set_active(true);
    else if (bNeg)
        mxRBNeg->set_active(true);

    sal_Int32 nTypePos = getTypePos(mxModel, aCID);
    mxLBType->set_active(nTypePos);

    if (nTypePos <= 1)
    {
        if (bPos)
            mxMFPos->set_sensitive(true);
        else
            mxMFPos->set_sensitive(false);

        if (bNeg)
            mxMFNeg->set_sensitive(true);
        else
            mxMFNeg->set_sensitive(false);

        double nValPos = getValue(mxModel, aCID, ErrorBarDirection::POSITIVE);
        double nValNeg = getValue(mxModel, aCID, ErrorBarDirection::NEGATIVE);

        mxMFPos->set_value(nValPos);
        mxMFNeg->set_value(nValNeg);
    }
    else
    {
        mxMFPos->set_sensitive(false);
        mxMFNeg->set_sensitive(false);
    }
}

std::unique_ptr<PanelLayout> ChartErrorBarPanel::Create (
    weld::Widget* pParent,
    ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(u"no parent Window given to ChartErrorBarPanel::Create"_ustr, nullptr, 0);
    return std::make_unique<ChartErrorBarPanel>(pParent, pController);
}

void ChartErrorBarPanel::DataChanged(const DataChangedEvent& rEvent)
{
    PanelLayout::DataChanged(rEvent);
    updateData();
}

void ChartErrorBarPanel::HandleContextChange(
    const vcl::EnumContext& )
{
    updateData();
}

void ChartErrorBarPanel::NotifyItemUpdate(
    sal_uInt16 /*nSID*/,
    SfxItemState /*eState*/,
    const SfxPoolItem* /*pState*/ )
{
}

void ChartErrorBarPanel::modelInvalid()
{
    mbModelValid = false;
}

void ChartErrorBarPanel::doUpdateModel(const rtl::Reference<::chart::ChartModel>& xModel)
{
    if (mbModelValid)
    {
        mxModel->removeModifyListener(mxListener);
    }

    mxModel = xModel;
    mbModelValid = mxModel.is();

    if (!mbModelValid)
        return;

    mxModel->addModifyListener(mxListener);
}

void ChartErrorBarPanel::updateModel(css::uno::Reference<css::frame::XModel> xModel)
{
    ::chart::ChartModel* pModel = dynamic_cast<::chart::ChartModel*>(xModel.get());
    assert(!xModel || pModel);
    doUpdateModel(pModel);
}

IMPL_LINK_NOARG(ChartErrorBarPanel, RadioBtnHdl, weld::Toggleable&, void)
{
    OUString aCID = getCID(mxModel);
    bool bPos = mxRBPosAndNeg->get_active() || mxRBPos->get_active();
    bool bNeg = mxRBPosAndNeg->get_active() || mxRBNeg->get_active();

    setShowPositiveError(mxModel, aCID, bPos);
    setShowNegativeError(mxModel, aCID, bNeg);
}

IMPL_LINK_NOARG(ChartErrorBarPanel, ListBoxHdl, weld::ComboBox&, void)
{
    OUString aCID = getCID(mxModel);
    sal_Int32 nPos = mxLBType->get_active();

    setTypePos(mxModel, aCID, nPos);
}

IMPL_LINK(ChartErrorBarPanel, NumericFieldHdl, weld::SpinButton&, rMetricField, void)
{
    OUString aCID = getCID(mxModel);
    double nVal = rMetricField.get_value();
    if (&rMetricField == mxMFPos.get())
        setValue(mxModel, aCID, nVal, ErrorBarDirection::POSITIVE);
    else if (&rMetricField == mxMFNeg.get())
        setValue(mxModel, aCID, nVal, ErrorBarDirection::NEGATIVE);
}

} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
