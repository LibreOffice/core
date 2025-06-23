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

#include <com/sun/star/frame/XFrame.hpp>

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include "ChartSidebarModifyListener.hxx"
#include <ChartModel.hxx>

class ToolbarUnoDispatcher;

namespace chart
{
class ChartController;

namespace sidebar
{
class ThemeWrapper;

class ChartThemePanel final : public PanelLayout,
                              public sfx2::sidebar::IContextChangeReceiver,
                              public sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface,
                              public sfx2::sidebar::SidebarModelUpdate,
                              public ChartSidebarModifyListenerParent
{
public:
    static std::unique_ptr<PanelLayout>
    Create(weld::Widget* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame,
           ChartController* pController);

    void DataChanged(const DataChangedEvent& rEvent) override;

    void HandleContextChange(const vcl::EnumContext& rContext) override;

    void NotifyItemUpdate(sal_uInt16 nSId, SfxItemState eState, const SfxPoolItem* pState) override;

    void GetControlState(const sal_uInt16 /*nSId*/,
                         boost::property_tree::ptree& /*rState*/) override
    {
    }

    // constructor/destructor
    ChartThemePanel(weld::Widget* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame,
                    ChartController* pController);
    ~ChartThemePanel() override;

    void updateData() override;
    void modelInvalid() override;

    void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

private:
    rtl::Reference<ChartModel> mxModel;
    ChartController* mpController;

    css::uno::Reference<css::util::XModifyListener> mxModifyListener;

    bool mbModelValid;

    std::unique_ptr<weld::Toolbar> mxThemeTB;
    std::unique_ptr<ToolbarUnoDispatcher> mxThemeDispatch;
    std::shared_ptr<ThemeWrapper> mxThemeWrapper;
    std::unique_ptr<weld::Button> mxSaveToNewTheme;

    DECL_LINK(ClickSaveToNewHdl, weld::Button&, void);

    void Initialize();
    void doUpdateModel(const rtl::Reference<ChartModel>& xModel);
};
} // end of namespace sidebar
} // end of namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
