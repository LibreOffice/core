/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTAREAPANEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTAREAPANEL_HXX

#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
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
#include <boost/scoped_ptr.hpp>

#include <svx/sidebar/AreaPropertyPanelBase.hxx>

#include "ChartSidebarModifyListener.hxx"

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

class ChartAreaPanel : public svx::sidebar::AreaPropertyPanelBase,
    public ChartSidebarModifyListenerParent
{
public:
    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);

    // constructor/destuctor
    ChartAreaPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);

    virtual ~ChartAreaPanel();

    virtual void setFillTransparence(const XFillTransparenceItem& rItem);
    virtual void setFillFloatTransparence(const XFillFloatTransparenceItem& rItem);
    virtual void setFillStyle(const XFillStyleItem& rItem);
    virtual void setFillStyleAndColor(const XFillStyleItem* pStyleItem, const XFillColorItem& rColorItem);
    virtual void setFillStyleAndGradient(const XFillStyleItem* pStyleItem, const XFillGradientItem& rGradientItem);
    virtual void setFillStyleAndHatch(const XFillStyleItem* pStyleItem, const XFillHatchItem& rHatchItem);
    virtual void setFillStyleAndBitmap(const XFillStyleItem* pStyleItem, const XFillBitmapItem& rBitmapItem);

    virtual void updateData() SAL_OVERRIDE;

    virtual void modelInvalid() SAL_OVERRIDE;

    virtual void dispose() SAL_OVERRIDE;

private:

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;

    void Initialize();

};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
