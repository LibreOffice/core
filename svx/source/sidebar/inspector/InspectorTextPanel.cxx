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
#include <vcl/window.hxx>
#include <vcl/cursor.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <bits/stdc++.h>
#include <sfx2/viewsh.hxx>
using namespace std;

#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace css;

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
    , mxListBoxStyles(m_xBuilder->weld_tree_view("listbox_fonts"))
    , maCharacterStyles(SID_STYLE_FAMILY1, *pBindings, *this)
    , maParagraphStyles(SID_STYLE_FAMILY2, *pBindings, *this)
    , mpBindings(pBindings)
{
    mxListBoxStyles->set_size_request(-1, mxListBoxStyles->get_height_rows(10));
}

InspectorTextPanel::~InspectorTextPanel() { disposeOnce(); }

void InspectorTextPanel::dispose()
{
    mxListBoxStyles.reset();
    maCharacterStyles.dispose();
    maParagraphStyles.dispose();
    PanelLayout::dispose();
}

void InspectorTextPanel::DataChanged(const DataChangedEvent&) {}

void InspectorTextPanel::NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState,
                                          const SfxPoolItem*)
{
    vcl::Cursor* pText;
    // pText = pParent->GetCursor();
    // if(pText)
    //     cout<<pText->GetSize();
    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (pViewShell)
    {
        vcl::Window* present = pViewShell->GetEditWindowForActiveOLEObj();
        if (present)
        {
            pText = present->GetCursor();
            if (pText)
            {
                cout << pText->GetSize() << "\n\n";
                cout << pText->GetStyle() << "\n\n";
            }
        }
    }
    switch (nSId)
    {
        case SID_STYLE_FAMILY1:
        {
            SfxObjectShell* pDocShell = SfxObjectShell::Current();
            // SfxObjectShell* pDocsShell = SfxEditShell::Current();
            SfxStyleSheetBasePool* pStyleSheetPool;
            pStyleSheetPool = pDocShell->GetStyleSheetPool();
            // OUString aStr = "Hi there";
            // SfxStyleSheetBase* pStyle = pStyleSheetPool->Find(aStr, SfxStyleFamily::Char);
            // cout << pStyle->GetName() << "\n\n";

            auto xIter = pStyleSheetPool->CreateIterator(
                SfxStyleFamily::Char, SfxStyleSearchBits::Used); // All the applied Paragraph styles
            SfxStyleSheetBase* StyleNames = nullptr;
            StyleNames = xIter->First();
            int cnt = 0;
            while (StyleNames)
            {
                OUString name = StyleNames->GetName();
                mxListBoxStyles->append_text(name);
                StyleNames = xIter->Next();
                ++cnt;
            }
        }
        break;
        case SID_STYLE_FAMILY2:
        {
            SfxObjectShell* pDocShell = SfxObjectShell::Current();
            SfxStyleSheetBasePool* pStyleSheetPool;
            pStyleSheetPool = pDocShell->GetStyleSheetPool();

            auto xIter = pStyleSheetPool->CreateIterator(
                SfxStyleFamily::Para, SfxStyleSearchBits::Used); // All the applied Paragraph styles
            SfxStyleSheetBase* StyleNames = nullptr;
            StyleNames = xIter->First();
            int cnt = 0;
            while (StyleNames)
            {
                OUString name = StyleNames->GetName();
                mxListBoxStyles->append_text(name);
                StyleNames = xIter->Next();
                ++cnt;
            }
        }
        break;
        default:
            break;
    }
}

void InspectorTextPanel::HandleContextChange(const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
        return;

    maContext = rContext;

    mxListBoxStyles->clear();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
