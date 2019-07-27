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

ScTpCompatOptions::ScTpCompatOptions(TabPageParent pParent, const SfxItemSet &rCoreAttrs)
    : SfxTabPage(pParent, "modules/scalc/ui/optcompatibilitypage.ui", "OptCompatibilityPage", &rCoreAttrs)
    , m_xLbKeyBindings(m_xBuilder->weld_combo_box("keybindings"))
{
}

ScTpCompatOptions::~ScTpCompatOptions()
{
}

VclPtr<SfxTabPage> ScTpCompatOptions::Create(TabPageParent pParent, const SfxItemSet *rCoreAttrs)
{
    return VclPtr<ScTpCompatOptions>::Create(pParent, *rCoreAttrs);
}

bool ScTpCompatOptions::FillItemSet(SfxItemSet *rCoreAttrs)
{
    bool bRet = false;
    if (m_xLbKeyBindings->get_value_changed_from_saved())
    {
        rCoreAttrs->Put(
            SfxUInt16Item(
                SID_SC_OPT_KEY_BINDING_COMPAT, m_xLbKeyBindings->get_active()));
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
                m_xLbKeyBindings->set_active(0);
            break;
            case ScOptionsUtil::KEY_OOO_LEGACY:
                m_xLbKeyBindings->set_active(1);
            break;
            default:
                ;
        }
    }

    m_xLbKeyBindings->save_value();
}

DeactivateRC ScTpCompatOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    return DeactivateRC::KeepPage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
