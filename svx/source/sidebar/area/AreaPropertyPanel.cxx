/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "AreaPropertyPanel.hxx"
#include <svx/svxids.hrc>
#include <svx/xfltrit.hxx>
#include <svx/xflftrit.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>


using namespace css;
using namespace css::uno;

namespace svx::sidebar {

AreaPropertyPanel::AreaPropertyPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
    : AreaPropertyPanelBase(pParent, rxFrame),
      maStyleControl(SID_ATTR_FILL_STYLE, *pBindings, *this),
      maColorControl(SID_ATTR_FILL_COLOR, *pBindings, *this),
      maGradientControl(SID_ATTR_FILL_GRADIENT, *pBindings, *this),
      maHatchControl(SID_ATTR_FILL_HATCH, *pBindings, *this),
      maBitmapControl(SID_ATTR_FILL_BITMAP, *pBindings, *this),
      maGradientListControl(SID_GRADIENT_LIST, *pBindings, *this),
      maHatchListControl(SID_HATCH_LIST, *pBindings, *this),
      maBitmapListControl(SID_BITMAP_LIST, *pBindings, *this),
      maPatternListControl(SID_PATTERN_LIST, *pBindings, *this),
      maFillTransparenceController(SID_ATTR_FILL_TRANSPARENCE, *pBindings, *this),
      maFillFloatTransparenceController(SID_ATTR_FILL_FLOATTRANSPARENCE, *pBindings, *this),
      mpBindings(pBindings)
{
}

AreaPropertyPanel::~AreaPropertyPanel()
{
    disposeOnce();
}

void AreaPropertyPanel::dispose()
{
    maStyleControl.dispose();
    maColorControl.dispose();
    maGradientControl.dispose();
    maHatchControl.dispose();
    maBitmapControl.dispose();
    maGradientListControl.dispose();
    maHatchListControl.dispose();
    maBitmapListControl.dispose();
    maPatternListControl.dispose();
    maFillTransparenceController.dispose();
    maFillFloatTransparenceController.dispose();

    AreaPropertyPanelBase::dispose();
}

VclPtr<PanelLayout> AreaPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to AreaPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to AreaPropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to AreaPropertyPanel::Create", nullptr, 2);

    return VclPtr<AreaPropertyPanel>::Create(
                pParent,
                rxFrame,
                pBindings);
}

void AreaPropertyPanel::setFillTransparence(const XFillTransparenceItem& rItem)
{
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_FILL_TRANSPARENCE,
            SfxCallMode::RECORD, { &rItem });
}

void AreaPropertyPanel::setFillFloatTransparence(const XFillFloatTransparenceItem& rItem)
{
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_FILL_FLOATTRANSPARENCE,
            SfxCallMode::RECORD, { &rItem });
}

void AreaPropertyPanel::setFillStyle(const XFillStyleItem& rItem)
{
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_FILL_STYLE,
            SfxCallMode::RECORD, { &rItem });
}

void AreaPropertyPanel::setFillStyleAndColor(const XFillStyleItem* pStyleItem,
        const XFillColorItem& rColorItem)
{
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_FILL_COLOR,
        SfxCallMode::RECORD, pStyleItem
            ? std::initializer_list<SfxPoolItem const*>{ &rColorItem, pStyleItem }
            : std::initializer_list<SfxPoolItem const*>{ &rColorItem });
}

void AreaPropertyPanel::setFillStyleAndGradient(const XFillStyleItem* pStyleItem,
        const XFillGradientItem& rGradientItem)
{
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_FILL_GRADIENT,
        SfxCallMode::RECORD, pStyleItem
            ? std::initializer_list<SfxPoolItem const*>{ &rGradientItem, pStyleItem }
            : std::initializer_list<SfxPoolItem const*>{ &rGradientItem });
}

void AreaPropertyPanel::setFillStyleAndHatch(const XFillStyleItem* pStyleItem,
        const XFillHatchItem& rHatchItem)
{
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_FILL_HATCH,
        SfxCallMode::RECORD, pStyleItem
            ? std::initializer_list<SfxPoolItem const*>{ &rHatchItem, pStyleItem }
            : std::initializer_list<SfxPoolItem const*>{ &rHatchItem });
}

void AreaPropertyPanel::setFillStyleAndBitmap(const XFillStyleItem* pStyleItem,
        const XFillBitmapItem& rBitmapItem)
{
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_FILL_BITMAP,
        SfxCallMode::RECORD, pStyleItem
            ? std::initializer_list<SfxPoolItem const*>{ &rBitmapItem, pStyleItem }
            : std::initializer_list<SfxPoolItem const*>{ &rBitmapItem });
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
