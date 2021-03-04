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
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>

#include <svx/sidebar/AreaPropertyPanelBase.hxx>

#include "ChartSidebarModifyListener.hxx"
#include "ChartSidebarSelectionListener.hxx"
#include "ChartColorWrapper.hxx"

class XFillFloatTransparenceItem;
class XFillTransparenceItem;
class XFillColorItem;

namespace chart {

class ChartController;

namespace sidebar {

class ChartAreaPanel : public svx::sidebar::AreaPropertyPanelBase,
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
    ChartAreaPanel(
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);

    virtual ~ChartAreaPanel() override;

    virtual void setFillTransparence(const XFillTransparenceItem& rItem) override;
    virtual void setFillFloatTransparence(const XFillFloatTransparenceItem& rItem) override;
    virtual void setFillStyle(const XFillStyleItem& rItem) override;
    virtual void setFillStyleAndColor(const XFillStyleItem* pStyleItem, const XFillColorItem& rColorItem) override;
    virtual void setFillStyleAndGradient(const XFillStyleItem* pStyleItem, const XFillGradientItem& rGradientItem) override;
    virtual void setFillStyleAndHatch(const XFillStyleItem* pStyleItem, const XFillHatchItem& rHatchItem) override;
    virtual void setFillStyleAndBitmap(const XFillStyleItem* pStyleItem, const XFillBitmapItem& rBitmapItem) override;

    virtual void updateData() override;
    virtual void modelInvalid() override;

    virtual void selectionChanged(bool bCorrectType) override;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

private:

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;
    rtl::Reference<ChartSidebarSelectionListener> mxSelectionListener;

    void Initialize();

    bool mbUpdate;
    bool mbModelValid;

    ChartColorWrapper maFillColorWrapper;
};

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
