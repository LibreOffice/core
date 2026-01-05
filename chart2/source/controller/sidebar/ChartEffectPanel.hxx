/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/sidebar/EffectPropertyPanelBase.hxx>
#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include "ChartSidebarModifyListener.hxx"
#include "ChartSidebarSelectionListener.hxx"

namespace chart
{
class ChartController;

namespace sidebar
{
class ChartEffectPanel : public svx::sidebar::EffectPropertyPanelBase,
                         public sfx2::sidebar::SidebarModelUpdate,
                         public ChartSidebarModifyListenerParent,
                         public ChartSidebarSelectionListenerParent
{
public:
    ChartEffectPanel(weld::Widget* pParent, ChartController* pController);
    ~ChartEffectPanel() override;

    static std::unique_ptr<PanelLayout> Create(weld::Widget* pParent, ChartController* pController);

    void setGlowRadius(const SdrMetricItem& rItem) override;
    void setGlowColor(const XColorItem& rItem) override;
    void setGlowTransparency(const SdrPercentItem& rItem) override;
    void setSoftEdgeRadius(const SdrMetricItem& rItem) override;

    void updateData() override;
    void modelInvalid() override;

    void selectionChanged(bool bCorrectType) override;

    void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

private:
    void Initialize();
    void doUpdateModel(const rtl::Reference<::chart::ChartModel>& xModel);
    // bool selectionIsDataSeries() const;

    rtl::Reference<::chart::ChartModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;
    rtl::Reference<ChartSidebarSelectionListener> mxSelectionListener;

    bool mbUpdate;
    bool mbModelValid;
};
}
} // end of namespace chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
