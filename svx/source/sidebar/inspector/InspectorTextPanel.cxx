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

#include "InspectorTextPanel.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>

using namespace css;

namespace svx::sidebar
{
VclPtr<vcl::Window>
InspectorTextPanel::Create(vcl::Window* pParent,
                           const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to InspectorTextPanel::Create",
                                             nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to InspectorTextPanel::Create",
                                             nullptr, 1);

    return VclPtr<InspectorTextPanel>::Create(pParent, rxFrame);
}

InspectorTextPanel::InspectorTextPanel(vcl::Window* pParent,
                                       const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, "InspectorTextPanel", "svx/ui/inspectortextpanel.ui", rxFrame)
    , mxFont(m_xBuilder->weld_toolbar("font"))
    , mxFontDispatch(new ToolbarUnoDispatcher(*mxFont, *m_xBuilder, rxFrame))
    , mxFontHeight(m_xBuilder->weld_toolbar("fontheight"))
    , mxFontHeightDispatch(new ToolbarUnoDispatcher(*mxFontHeight, *m_xBuilder, rxFrame))
{
}

InspectorTextPanel::~InspectorTextPanel() { disposeOnce(); }

void InspectorTextPanel::dispose()
{
    mxFontHeightDispatch.reset();
    mxFontDispatch.reset();

    mxFontHeight.reset();
    mxFont.reset();

    PanelLayout::dispose();
}

void InspectorTextPanel::HandleContextChange(const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
        return;

    maContext = rContext;
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
