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
#include <officecfg/Office/Calc.hxx>

#include <tpcompatibility.hxx>
#include <sc.hrc>
#include <optutil.hxx>

ScTpCompatOptions::ScTpCompatOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rCoreAttrs)
    : SfxTabPage(pPage, pController, u"modules/scalc/ui/optcompatibilitypage.ui"_ustr, u"OptCompatibilityPage"_ustr, &rCoreAttrs)
    , m_xLbKeyBindings(m_xBuilder->weld_combo_box(u"keybindings"_ustr))
    , m_xLbKeyBindingsImg(m_xBuilder->weld_widget(u"lockkeybindings"_ustr))
    , m_xBtnLink(m_xBuilder->weld_check_button(u"cellLinkCB"_ustr))
    , m_xBtnLinkImg(m_xBuilder->weld_widget(u"lockcellLinkCB"_ustr))
{
}

ScTpCompatOptions::~ScTpCompatOptions()
{
}

std::unique_ptr<SfxTabPage> ScTpCompatOptions::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rCoreAttrs)
{
    return std::make_unique<ScTpCompatOptions>(pPage, pController, *rCoreAttrs);
}

OUString ScTpCompatOptions::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"label2"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    // id "keybindings" GtkComboBoxText is not included

    return sAllStrings.replaceAll("_", "");
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
    m_xLbKeyBindings->set_sensitive(!officecfg::Office::Calc::Compatibility::KeyBindings::BaseGroup::isReadOnly());
    m_xLbKeyBindingsImg->set_visible(officecfg::Office::Calc::Compatibility::KeyBindings::BaseGroup::isReadOnly());
    m_xLbKeyBindings->save_value();

    if (const SfxBoolItem* pbItem = rCoreAttrs->GetItemIfSet(SID_SC_OPT_LINKS))
    {
        m_xBtnLink->set_active(pbItem->GetValue());
    }
    m_xBtnLink->set_sensitive(!officecfg::Office::Calc::Compatibility::Links::isReadOnly());
    m_xBtnLinkImg->set_visible(officecfg::Office::Calc::Compatibility::Links::isReadOnly());
    m_xBtnLink->save_state();
}

DeactivateRC ScTpCompatOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    return DeactivateRC::KeepPage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
