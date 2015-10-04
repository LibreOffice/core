/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTERRORBARPANEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTERRORBARPANEL_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include <svx/sidebar/PanelLayout.hxx>

#include "ChartSidebarModifyListener.hxx"

#include <com/sun/star/util/XModifyListener.hpp>

class FixedText;
class ListBox;
class NumericField;
class MetricField;

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
    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);

    virtual void DataChanged(
        const DataChangedEvent& rEvent) SAL_OVERRIDE;

    virtual void HandleContextChange(
        const ::sfx2::sidebar::EnumContext& rContext) SAL_OVERRIDE;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) SAL_OVERRIDE;

    // constructor/destuctor
    ChartErrorBarPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);
    virtual ~ChartErrorBarPanel();
    virtual void dispose() SAL_OVERRIDE;

    virtual void updateData() SAL_OVERRIDE;
    virtual void modelInvalid() SAL_OVERRIDE;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) SAL_OVERRIDE;

private:
    //ui controls
    VclPtr<RadioButton> mpRBPosAndNeg;
    VclPtr<RadioButton> mpRBPos;
    VclPtr<RadioButton> mpRBNeg;

    VclPtr<ListBox> mpLBType;

    VclPtr<NumericField> mpMFPos;
    VclPtr<NumericField> mpMFNeg;

    css::uno::Reference<css::frame::XFrame> mxFrame;

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;

    bool mbModelValid;

    void Initialize();

    DECL_LINK_TYPED(RadioBtnHdl, RadioButton&, void);
    DECL_LINK_TYPED(ListBoxHdl, ListBox&, void);
    DECL_LINK(NumericFieldHdl, NumericField*);
};

} } // end of namespace ::chart::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
