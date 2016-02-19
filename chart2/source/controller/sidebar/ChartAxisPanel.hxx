/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTAXISPANEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTAXISPANEL_HXX

#include <sfx2/abstractbar/ControllerItem.hxx>
#include <sfx2/abstractbar/IContextChangeReceiver.hxx>
#include <sfx2/abstractbar/SidebarModelUpdate.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/layout.hxx>

#include "ChartSidebarModifyListener.hxx"
#include "ChartSidebarSelectionListener.hxx"

#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>

class FixedText;
class ListBox;
class MetricField;

namespace chart {

class ChartController;

namespace sidebar {

class ChartAxisPanel : public PanelLayout,
    public ::sfx2::abstractbar::IContextChangeReceiver,
    public ::sfx2::abstractbar::ControllerItem::ItemUpdateReceiverInterface,
    public sfx2::abstractbar::SidebarModelUpdate,
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
        const ::sfx2::abstractbar::EnumContext& rContext) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) override;

    // constructor/destuctor
    ChartAxisPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);
    virtual ~ChartAxisPanel();
    virtual void dispose() override;

    virtual void updateData() override;
    virtual void modelInvalid() override;

    virtual void selectionChanged(bool bCorrectType) override;
    virtual void SelectionInvalid() override;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

private:
    //ui controls
    VclPtr<CheckBox> mpCBShowLabel;
    VclPtr<CheckBox> mpCBReverse;

    VclPtr<ListBox> mpLBLabelPos;
    VclPtr<VclGrid> mpGridLabel;

    VclPtr<MetricField> mpNFRotation;

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxModifyListener;
    css::uno::Reference<css::view::XSelectionChangeListener> mxSelectionListener;

    bool mbModelValid;

    void Initialize();

    DECL_LINK_TYPED(CheckBoxHdl, Button*, void);
    DECL_LINK_TYPED(ListBoxHdl, ListBox&, void);
    DECL_LINK_TYPED(TextRotationHdl, Edit&, void);
};

} } // end of namespace ::chart::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
