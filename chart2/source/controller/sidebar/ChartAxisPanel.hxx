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
    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
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
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);
    virtual ~ChartAxisPanel() override;
    virtual void dispose() override;

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

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxModifyListener;
    css::uno::Reference<css::view::XSelectionChangeListener> mxSelectionListener;

    bool mbModelValid;

    void Initialize();

    DECL_LINK(CheckBoxHdl, weld::ToggleButton&, void);
    DECL_LINK(ListBoxHdl, weld::ComboBox&, void);
    DECL_LINK(TextRotationHdl, weld::MetricSpinButton&, void);
};

} } // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
