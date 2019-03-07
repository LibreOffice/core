/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#undef SC_DLLIMPLEMENTATION

#include <svl/intitem.hxx>

#include <tpcompatibility.hxx>
#include <sc.hrc>
#include <optutil.hxx>

ScTpCompatOptions::ScTpCompatOptions(vcl::Window *pParent, const SfxItemSet &rCoreAttrs) :
    SfxTabPage(pParent, "OptCompatibilityPage","modules/scalc/ui/optcompatibilitypage.ui", &rCoreAttrs)
{
    get(m_pLbKeyBindings,"keybindings");
}

ScTpCompatOptions::~ScTpCompatOptions()
{
    disposeOnce();
}

void ScTpCompatOptions::dispose()
{
    m_pLbKeyBindings.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> ScTpCompatOptions::Create(TabPageParent pParent, const SfxItemSet *rCoreAttrs)
{
    return VclPtr<ScTpCompatOptions>::Create(pParent.pParent, *rCoreAttrs);
}

bool ScTpCompatOptions::FillItemSet(SfxItemSet *rCoreAttrs)
{
    bool bRet = false;
    if (m_pLbKeyBindings->IsValueChangedFromSaved())
    {
        rCoreAttrs->Put(
            SfxUInt16Item(
                SID_SC_OPT_KEY_BINDING_COMPAT, m_pLbKeyBindings->GetSelectedEntryPos()));
        bRet = true;
    }
    return bRet;
}

void ScTpCompatOptions::Reset(const SfxItemSet *rCoreAttrs)
{
    const SfxPoolItem* pItem;
    if (rCoreAttrs->HasItem(SID_SC_OPT_KEY_BINDING_COMPAT, &pItem))
    {
        const SfxUInt16Item* p16Item = static_cast<const SfxUInt16Item*>(pItem);
        ScOptionsUtil::KeyBindingType eKeyB =
            static_cast<ScOptionsUtil::KeyBindingType>(p16Item->GetValue());

        switch (eKeyB)
        {
            case ScOptionsUtil::KEY_DEFAULT:
                m_pLbKeyBindings->SelectEntryPos(0);
            break;
            case ScOptionsUtil::KEY_OOO_LEGACY:
                m_pLbKeyBindings->SelectEntryPos(1);
            break;
            default:
                ;
        }
    }

    m_pLbKeyBindings->SaveValue();
}

DeactivateRC ScTpCompatOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    return DeactivateRC::KeepPage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
