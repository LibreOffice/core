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

namespace com::sun::star::util { class XModifyListener; }

namespace chart {

class ChartController;

namespace sidebar {

class ChartErrorBarPanel : public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface,
    public sfx2::sidebar::SidebarModelUpdate,
    public ChartSidebarModifyListenerParent
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
    ChartErrorBarPanel(
        weld::Widget* pParent,
        ChartController* pController);
    virtual ~ChartErrorBarPanel() override;

    virtual void updateData() override;
    virtual void modelInvalid() override;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

private:
    //ui controls
    std::unique_ptr<weld::RadioButton> mxRBPosAndNeg;
    std::unique_ptr<weld::RadioButton> mxRBPos;
    std::unique_ptr<weld::RadioButton> mxRBNeg;

    std::unique_ptr<weld::ComboBox> mxLBType;

    std::unique_ptr<weld::SpinButton> mxMFPos;
    std::unique_ptr<weld::SpinButton> mxMFNeg;

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;

    bool mbModelValid;

    void Initialize();

    DECL_LINK(RadioBtnHdl, weld::ToggleButton&, void);
    DECL_LINK(ListBoxHdl, weld::ComboBox&, void);
    DECL_LINK(NumericFieldHdl, weld::SpinButton&, void);
};

} } // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
