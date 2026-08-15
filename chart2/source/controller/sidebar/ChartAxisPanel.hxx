/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/weld/CheckButton.hxx>
#include <vcl/weld/ComboBox.hxx>
#include <vcl/weld/FormattedSpinButton.hxx>
#include <vcl/weld/MetricSpinButton.hxx>
#include <vcl/weld/RadioButton.hxx>
#include "ChartSidebarModifyListener.hxx"
#include "ChartSidebarSelectionListener.hxx"

namespace com::sun::star::util { class XModifyListener; }
namespace com::sun::star::view { class XSelectionChangeListener; }

namespace chart {

class ChartController;

namespace sidebar {

class ChartAxisPanel : public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface,
    public sfx2::sidebar::SidebarModelUpdate,
    public ChartSidebarModifyListenerParent,
    public ChartSidebarSelectionListenerParent
{
public:
    static std::unique_ptr<PanelLayout> Create(
        weld::Widget* pParent,
        ChartController* pController);

    virtual void DataChanged(
        const DataChangedEvent& rEvent) override;

    virtual void HandleContextChange(
        const vcl::EnumContext& rContext) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState) override;

    virtual void GetControlState(
        const sal_uInt16 /*nSId*/,
        boost::property_tree::ptree& /*rState*/) override {};

    // constructor/destructor
    ChartAxisPanel(
        weld::Widget* pParent,
        ChartController* pController);
    virtual ~ChartAxisPanel() override;

    virtual void updateData() override;
    virtual void modelInvalid() override;

    virtual void selectionChanged(bool bCorrectType) override;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

private:
    //ui controls
    std::unique_ptr<weld::CheckButton> mxCBShowLabel;
    std::unique_ptr<weld::CheckButton> mxCBReverse;
    std::unique_ptr<weld::ComboBox> mxLBLabelPos;
    std::unique_ptr<weld::Widget> mxGridLabel;
    std::unique_ptr<weld::MetricSpinButton> mxNFRotation;

    std::unique_ptr<weld::Widget> mxBxHistogramBinning;
    std::unique_ptr<weld::RadioButton> mxRBHistogramAutomatic;
    std::unique_ptr<weld::RadioButton> mxRBHistogramBinWidth;
    std::unique_ptr<weld::FormattedSpinButton> mxHistogramBinWidth;
    std::unique_ptr<weld::RadioButton> mxRBHistogramBinCount;
    std::unique_ptr<weld::SpinButton> mxHistogramBinCount;
    std::unique_ptr<weld::CheckButton> mxCBHistogramOverflow;
    std::unique_ptr<weld::FormattedSpinButton> mxHistogramOverflow;
    std::unique_ptr<weld::CheckButton> mxCBHistogramUnderflow;
    std::unique_ptr<weld::FormattedSpinButton> mxHistogramUnderflow;

    rtl::Reference<::chart::ChartModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxModifyListener;
    css::uno::Reference<css::view::XSelectionChangeListener> mxSelectionListener;

    bool mbModelValid;
    // True while the panel is writing widget values from the model.
    bool mbUpdating;

    void Initialize();
    void doUpdateModel(const rtl::Reference<::chart::ChartModel>& xModel);
    void updateHistogramControlSensitivity();

    DECL_LINK(CheckBoxHdl, weld::Toggleable&, void);
    DECL_LINK(ListBoxHdl, weld::ComboBox&, void);
    DECL_LINK(TextRotationHdl, weld::MetricSpinButton&, void);
    DECL_LINK(HistogramToggleHdl, weld::Toggleable&, void);
    DECL_LINK(HistogramValueHdl, weld::FormattedSpinButton&, void);
    DECL_LINK(HistogramBinCountHdl, weld::SpinButton&, void);
};

} } // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
