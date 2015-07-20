/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ChartAreaPanel.hxx"

#include "ChartController.hxx"

namespace chart { namespace sidebar {

VclPtr<vcl::Window> ChartAreaPanel::Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController)
{
    if (pParent == NULL)
        throw css::lang::IllegalArgumentException("no parent Window given to ChartAxisPanel::Create", NULL, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to ChartAxisPanel::Create", NULL, 1);

    return VclPtr<ChartAreaPanel>::Create(
                        pParent, rxFrame, pController);
}

ChartAreaPanel::ChartAreaPanel(vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* /*pController*/):
    svx::sidebar::AreaPropertyPanelBase(pParent, rxFrame)
{
}

ChartAreaPanel::~ChartAreaPanel()
{
    disposeOnce();
}

void ChartAreaPanel::setFillTransparence(const XFillTransparenceItem& /*rItem*/)
{

}

void ChartAreaPanel::setFillFloatTransparence(const XFillFloatTransparenceItem& /*rItem*/)
{

}

void ChartAreaPanel::setFillStyle(const XFillStyleItem& /*rItem*/)
{

}

void ChartAreaPanel::setFillStyleAndColor(const XFillStyleItem* /*pStyleItem*/,
        const XFillColorItem& /*rColorItem*/)
{

}

void ChartAreaPanel::setFillStyleAndGradient(const XFillStyleItem* /*pStyleItem*/,
        const XFillGradientItem& /*rGradientItem*/)
{

}

void ChartAreaPanel::setFillStyleAndHatch(const XFillStyleItem* /*pStyleItem*/,
        const XFillHatchItem& /*rHatchItem*/)
{

}

void ChartAreaPanel::setFillStyleAndBitmap(const XFillStyleItem* /*pStyleItem*/,
        const XFillBitmapItem& /*rBitmapItem*/)
{

}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
