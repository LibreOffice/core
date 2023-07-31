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
#include <svl/eitem.hxx>

#include <tpcompatibility.hxx>
#include <sc.hrc>
#include <optutil.hxx>

ScTpCompatOptions::ScTpCompatOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rCoreAttrs)
    : SfxTabPage(pPage, pController, "modules/scalc/ui/optcompatibilitypage.ui", "OptCompatibilityPage", &rCoreAttrs)
    , m_xLbKeyBindings(m_xBuilder->weld_combo_box("keybindings"))
    , m_xBtnLink(m_xBuilder->weld_check_button("cellLinkCB"))
{
}

ScTpCompatOptions::~ScTpCompatOptions()
{
}

std::unique_ptr<SfxTabPage> ScTpCompatOptions::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rCoreAttrs)
{
    return std::make_unique<ScTpCompatOptions>(pPage, pController, *rCoreAttrs);
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
    if (m_xBtnLink->get_state_changed_from_saved())
    {
        rCoreAttrs->Put(SfxBoolItem(SID_SC_OPT_LINKS, m_xBtnLink->get_active()));
        bRet = true;
    }

    return bRet;
}

void ScTpCompatOptions::Reset(const SfxItemSet *rCoreAttrs)
{
    if (const SfxUInt16Item* p16Item = rCoreAttrs->GetItemIfSet(SID_SC_OPT_KEY_BINDING_COMPAT))
    {
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

    if (const SfxBoolItem* pbItem = rCoreAttrs->GetItemIfSet(SID_SC_OPT_LINKS))
    {
        m_xBtnLink->set_active(pbItem->GetValue());
    }
    m_xBtnLink->save_state();
}

DeactivateRC ScTpCompatOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    return DeactivateRC::KeepPage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
