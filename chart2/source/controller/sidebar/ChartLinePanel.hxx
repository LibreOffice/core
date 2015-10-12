/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTLINEPANEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTLINEPANEL_HXX

#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include <svx/xgrad.hxx>
#include <svx/itemwin.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/drawitem.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <svl/intitem.hxx>
#include <com/sun/star/ui/XUIElement.hpp>

#include <svx/sidebar/LinePropertyPanelBase.hxx>

#include "ChartSidebarModifyListener.hxx"
#include "ChartSidebarSelectionListener.hxx"
#include "ChartColorWrapper.hxx"

class XFillFloatTransparenceItem;
class XFillTransparenceItem;
class XFillStyleItem;
class XFillGradientItem;
class XFillColorItem;
class XFillHatchItem;
class XFillBitmapItem;

namespace chart {

class ChartController;

namespace sidebar {

class ChartLinePanel : public svx::sidebar::LinePropertyPanelBase,
    public sfx2::sidebar::SidebarModelUpdate,
    public ChartSidebarModifyListenerParent,
    public ChartSidebarSelectionListenerParent
{
public:
    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);

    // constructor/destuctor
    ChartLinePanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);

    virtual ~ChartLinePanel();

    virtual void updateData() override;
    virtual void modelInvalid() override;

    virtual void selectionChanged(bool bCorrectType) override;
    virtual void SelectionInvalid() override;

    virtual void dispose() override;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

    virtual void setLineWidth(const XLineWidthItem& rItem) override;

protected:

    virtual void setLineStyle(const XLineStyleItem& rItem) override;
    virtual void setLineDash(const XLineDashItem& rItem) override;
    virtual void setLineEndStyle(const XLineEndItem* pItem) override;
    virtual void setLineStartStyle(const XLineStartItem* pItem) override;
    virtual void setLineTransparency(const XLineTransparenceItem& rItem) override;
    virtual void setLineJoint(const XLineJointItem* pItem) override;
    virtual void setLineCap(const XLineCapItem* pItem) override;

private:

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;
    rtl::Reference<ChartSidebarSelectionListener> mxSelectionListener;

    void Initialize();

    bool mbUpdate;
    bool mbModelValid;
    ChartColorWrapper maLineColorWrapper;
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
