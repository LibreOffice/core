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

#include "LinePropertyPanel.hxx"
#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlinjoit.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace css;
using namespace css::uno;

namespace svx::sidebar {

LinePropertyPanel::LinePropertyPanel(
    weld::Widget* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   LinePropertyPanelBase(pParent, rxFrame),
    maStyleControl(SID_ATTR_LINE_STYLE, *pBindings, *this),
    maDashControl (SID_ATTR_LINE_DASH, *pBindings, *this),
    maWidthControl(SID_ATTR_LINE_WIDTH, *pBindings, *this),
    maTransControl(SID_ATTR_LINE_TRANSPARENCE, *pBindings, *this),
    maEdgeStyle(SID_ATTR_LINE_JOINT, *pBindings, *this),
    maCapStyle(SID_ATTR_LINE_CAP, *pBindings, *this),
    mpBindings(pBindings),
    maContext()
{
    setMapUnit(maWidthControl.GetCoreMetric());
}

LinePropertyPanel::~LinePropertyPanel()
{
    maStyleControl.dispose();
    maDashControl.dispose();
    maWidthControl.dispose();
    maTransControl.dispose();
    maEdgeStyle.dispose();
    maCapStyle.dispose();
}

std::unique_ptr<PanelLayout> LinePropertyPanel::Create (
    weld::Widget* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to LinePropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to LinePropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to LinePropertyPanel::Create", nullptr, 2);

    return std::make_unique<LinePropertyPanel>(pParent, rxFrame, pBindings);
}

void LinePropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
{
    const bool bDisabled(SfxItemState::DISABLED == eState);
    const bool bSetOrDefault = eState >= SfxItemState::DEFAULT;

    switch(nSID)
    {
        case SID_ATTR_LINE_TRANSPARENCE:
        {
            updateLineTransparence(bDisabled, bSetOrDefault, pState);
            break;
        }
        case SID_ATTR_LINE_WIDTH:
        {
            updateLineWidth(bDisabled, bSetOrDefault, pState);
            break;
        }
        case SID_ATTR_LINE_JOINT:
        {
            updateLineJoint(bDisabled, bSetOrDefault, pState);
            break;
        }
        case SID_ATTR_LINE_CAP:
        {
            updateLineCap(bDisabled, bSetOrDefault, pState);
            break;
        }
    }
    ActivateControls();
}

void LinePropertyPanel::HandleContextChange(
    const vcl::EnumContext& rContext)
{
    if(maContext == rContext)
    {
        // Nothing to do
        return;
    }

    maContext = rContext;
    bool bShowArrows = false;

    switch(maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application::Calc, Context::DrawLine):
        case CombinedEnumContext(Application::DrawImpress, Context::DrawLine):
        case CombinedEnumContext(Application::DrawImpress, Context::Draw):
        case CombinedEnumContext(Application::WriterVariants, Context::Draw):
            // TODO : Implement DrawLine context in Writer
            bShowArrows = true;
            break;
    }

    if (!bShowArrows)
        disableArrowHead();
    else
        enableArrowHead();
}

void LinePropertyPanel::setLineJoint(const XLineJointItem* pItem)
{
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_LINE_JOINT,
            SfxCallMode::RECORD, { pItem });
}

void LinePropertyPanel::setLineCap(const XLineCapItem* pItem)
{
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_LINE_CAP,
            SfxCallMode::RECORD, { pItem });
}

void LinePropertyPanel::setLineTransparency(const XLineTransparenceItem& rItem)
{
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_LINE_STYLE,
            SfxCallMode::RECORD, { &rItem });
}

void LinePropertyPanel::setLineWidth(const XLineWidthItem& rItem)
{
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_LINE_WIDTH,
            SfxCallMode::RECORD, { &rItem });
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
