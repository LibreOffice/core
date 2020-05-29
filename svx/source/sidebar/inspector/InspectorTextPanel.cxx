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

#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/weldutils.hxx>
#include <svx/svxids.hrc>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace css;

using namespace css::uno;

namespace svx::sidebar
{
VclPtr<vcl::Window>
InspectorTextPanel::Create(vcl::Window* pParent,
                           const css::uno::Reference<css::frame::XFrame>& rxFrame,
                           SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to InspectorTextPanel::Create",
                                             nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to InspectorTextPanel::Create",
                                             nullptr, 1);

    return VclPtr<InspectorTextPanel>::Create(pParent, rxFrame, pBindings);
}

InspectorTextPanel::InspectorTextPanel(vcl::Window* pParent,
                                       const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                       SfxBindings* pBindings)
    : PanelLayout(pParent, "InspectorTextPanel", "svx/ui/inspectortextpanel.ui", rxFrame)
    , mxproperty_1(m_xBuilder->weld_label("property1"))
    , mxproperty_2(m_xBuilder->weld_label("property2"))
    , mxproperty_3(m_xBuilder->weld_label("property3"))
    , mxproperty_4(m_xBuilder->weld_label("property4"))
    , mxproperty_5(m_xBuilder->weld_label("property5"))
    , mxproperty_6(m_xBuilder->weld_label("property6"))
    , maParagraphStyle(SID_STYLE_FAMILY, *pBindings, *this)
    , maCharacterStyle(SID_ATTR_CHAR, *pBindings, *this)
    , mpBindings(pBindings)
{
}

InspectorTextPanel::~InspectorTextPanel() { disposeOnce(); }

void InspectorTextPanel::dispose()
{
    mxproperty_1.reset();
    mxproperty_2.reset();
    mxproperty_3.reset();
    mxproperty_4.reset();
    mxproperty_5.reset();
    mxproperty_6.reset();

    maParagraphStyle.dispose();
    maCharacterStyle.dispose();

    PanelLayout::dispose();
}

void InspectorTextPanel::DataChanged(const DataChangedEvent&) {}

void InspectorTextPanel::NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                                          const SfxPoolItem* pState)
{
    SfxObjectShell* pDocShell = SfxObjectShell::Current();
    SfxStyleSheetBasePool* pStyleSheetPool;
    pStyleSheetPool = pDocShell->GetStyleSheetPool();

    auto xIter = pStyleSheetPool->CreateIterator(
        SfxStyleFamily::Para, SfxStyleSearchBits::Used); // All the applied Paragraph styles
    SfxStyleSheetBase* StyleNames = nullptr;
    StyleNames = xIter->First();
    while (StyleNames)
    {
        OUString name = StyleNames->GetName();
        property_dump[idx++]->set_label(name);
        StyleNames = xIter->Next();
    }

    xIter = pStyleSheetPool->CreateIterator(
        SfxStyleFamily::Char, SfxStyleSearchBits::Used); // All the applied Character styles
    StyleNames = xIter->First();
    while (StyleNames)
    {
        OUString name = StyleNames->GetName();
        property_dump[idx++]->set_label(name);
        StyleNames = xIter->Next();
    }
}

void InspectorTextPanel::HandleContextChange(const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
        return;

    maContext = rContext;
}
} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
