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
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <LinePropertyPanel.hxx>
#include <LinePropertyPanel.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/xtable.hxx>
#include <svx/xdash.hxx>
#include <svx/drawitem.hxx>
#include <svx/svxitems.hrc>
#include <svtools/valueset.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <vcl/svapp.hxx>
#include <svx/xlnwtit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/toolbox.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlinjoit.hxx>
#include "svx/sidebar/PopupContainer.hxx"
#include "svx/sidebar/PopupControl.hxx"
#include "LineWidthControl.hxx"
#include <boost/bind.hpp>

using namespace css;
using namespace css::uno;
using sfx2::sidebar::Theme;

namespace svx { namespace sidebar {

LinePropertyPanel::LinePropertyPanel(
    vcl::Window* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   LinePropertyPanelBase(pParent, rxFrame),
    maStyleControl(SID_ATTR_LINE_STYLE, *pBindings, *this),
    maDashControl (SID_ATTR_LINE_DASH, *pBindings, *this),
    maWidthControl(SID_ATTR_LINE_WIDTH, *pBindings, *this),
    maStartControl(SID_ATTR_LINE_START, *pBindings, *this),
    maEndControl(SID_ATTR_LINE_END, *pBindings, *this),
    maLineEndListControl(SID_LINEEND_LIST, *pBindings, *this),
    maLineStyleListControl(SID_DASH_LIST, *pBindings, *this),
    maTransControl(SID_ATTR_LINE_TRANSPARENCE, *pBindings, *this),
    maEdgeStyle(SID_ATTR_LINE_JOINT, *pBindings, *this),
    maCapStyle(SID_ATTR_LINE_CAP, *pBindings, *this),
    mpBindings(pBindings)
{
    Initialize();
}

LinePropertyPanel::~LinePropertyPanel()
{
    disposeOnce();
}

void LinePropertyPanel::dispose()
{
    maStyleControl.dispose();
    maDashControl.dispose();
    maWidthControl.dispose();
    maStartControl.dispose();
    maEndControl.dispose();
    maLineEndListControl.dispose();
    maLineStyleListControl.dispose();
    maTransControl.dispose();
    maEdgeStyle.dispose();
    maCapStyle.dispose();

    LinePropertyPanelBase::dispose();
}

void LinePropertyPanel::Initialize()
{
    setMapUnit(maWidthControl.GetCoreMetric());
}

VclPtr<vcl::Window> LinePropertyPanel::Create (
    vcl::Window* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to LinePropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to LinePropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to LinePropertyPanel::Create", nullptr, 2);

    return VclPtr<LinePropertyPanel>::Create(pParent, rxFrame, pBindings);
}

void LinePropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool /*bIsEnabled*/)
{
    const bool bDisabled(SfxItemState::DISABLED == eState);
    const bool bSetOrDefault = eState >= SfxItemState::DEFAULT;

    switch(nSID)
    {
        case SID_ATTR_LINE_DASH:
        {
            updateLineDash(bDisabled, bSetOrDefault, pState);
            break;
        }
        case SID_ATTR_LINE_STYLE:
        {
            updateLineStyle(bDisabled, bSetOrDefault, pState);
            break;
        }
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
        case SID_ATTR_LINE_START:
        {
            updateLineStart(bDisabled, bSetOrDefault, pState);
            break;
        }
        case SID_ATTR_LINE_END:
        {
            updateLineEnd(bDisabled, bSetOrDefault, pState);
            break;
        }
        case SID_LINEEND_LIST:
        {
            FillLineEndList();
            SelectEndStyle(true);
            SelectEndStyle(false);
            break;
        }
        case SID_DASH_LIST:
        {
            FillLineStyleList();
            SelectLineStyle();
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

void LinePropertyPanel::setLineStyle(const XLineStyleItem& rItem)
{
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_STYLE, SfxCallMode::RECORD, &rItem, 0L);
}

void LinePropertyPanel::setLineDash(const XLineDashItem& rItem)
{
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_DASH, SfxCallMode::RECORD, &rItem, 0L);
}

void LinePropertyPanel::setLineEndStyle(const XLineEndItem* pItem)
{
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINEEND_STYLE, SfxCallMode::RECORD, pItem,  0L);
}

void LinePropertyPanel::setLineStartStyle(const XLineStartItem* pItem)
{
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINEEND_STYLE, SfxCallMode::RECORD, pItem,  0L);
}

void LinePropertyPanel::setLineJoint(const XLineJointItem* pItem)
{
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_JOINT, SfxCallMode::RECORD, pItem,  0L);
}

void LinePropertyPanel::setLineCap(const XLineCapItem* pItem)
{
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_CAP, SfxCallMode::RECORD, pItem,  0L);
}

void LinePropertyPanel::setLineTransparency(const XLineTransparenceItem& rItem)
{
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_STYLE, SfxCallMode::RECORD, &rItem, 0L);
}

void LinePropertyPanel::setLineWidth(const XLineWidthItem& rItem)
{
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_WIDTH, SfxCallMode::RECORD, &rItem, 0L);
}

}} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
