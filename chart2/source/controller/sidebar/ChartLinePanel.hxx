/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include <svx/sidebar/LinePropertyPanelBase.hxx>

#include "ChartSidebarModifyListener.hxx"
#include "ChartSidebarSelectionListener.hxx"
#include "ChartColorWrapper.hxx"

class XLineCapItem;
class XLineDashItem;
class XLineEndItem;
class XLineJointItem;
class XLineStartItem;
class XLineStyleItem;
class XLineTransparenceItem;
class XLineWidthItem;

namespace chart {

class ChartController;

namespace sidebar {

class ChartLinePanel : public svx::sidebar::LinePropertyPanelBase,
    public sfx2::sidebar::SidebarModelUpdate,
    public ChartSidebarModifyListenerParent,
    public ChartSidebarSelectionListenerParent
{
public:
    static std::unique_ptr<PanelLayout> Create(
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);

    // constructor/destructor
    ChartLinePanel(
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);

    virtual ~ChartLinePanel() override;

    virtual void updateData() override;
    virtual void modelInvalid() override;

    virtual void selectionChanged(bool bCorrectType) override;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

    virtual void setLineWidth(const XLineWidthItem& rItem) override;

protected:

    virtual void setLineTransparency(const XLineTransparenceItem& rItem) override;
    virtual void setLineJoint(const XLineJointItem* pItem) override;
    virtual void setLineCap(const XLineCapItem* pItem) override;

    virtual void updateLineWidth(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem) override;

private:

    rtl::Reference<::chart::ChartModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;
    rtl::Reference<ChartSidebarSelectionListener> mxSelectionListener;

    void Initialize();
    void doUpdateModel(rtl::Reference<::chart::ChartModel> xModel);

    bool mbUpdate;
    bool mbModelValid;
    ChartColorWrapper maLineColorWrapper;
    ChartLineStyleWrapper maLineStyleWrapper;
};

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
