/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#undef SC_DLLIMPLEMENTATION

#include <tpdefaults.hxx>
#include <sc.hrc>
#include <defaultsoptions.hxx>
#include <document.hxx>

ScTpDefaultsOptions::ScTpDefaultsOptions(TabPageParent pParent, const SfxItemSet &rCoreSet)
    : SfxTabPage(pParent, "modules/scalc/ui/optdefaultpage.ui", "OptDefaultPage", &rCoreSet)
    , m_xEdNSheets(m_xBuilder->weld_spin_button("sheetsnumber"))
    , m_xEdSheetPrefix(m_xBuilder->weld_entry("sheetprefix"))
{
    m_xEdNSheets->connect_changed( LINK(this, ScTpDefaultsOptions, NumModifiedHdl) );
    m_xEdSheetPrefix->connect_changed( LINK(this, ScTpDefaultsOptions, PrefixModifiedHdl) );
    m_xEdSheetPrefix->connect_focus_in( LINK(this, ScTpDefaultsOptions, PrefixEditOnFocusHdl) );
}

ScTpDefaultsOptions::~ScTpDefaultsOptions()
{
}

VclPtr<SfxTabPage> ScTpDefaultsOptions::Create(TabPageParent pParent, const SfxItemSet *rCoreAttrs)
{
    return VclPtr<ScTpDefaultsOptions>::Create(pParent, *rCoreAttrs);
}

bool ScTpDefaultsOptions::FillItemSet(SfxItemSet *rCoreSet)
{
    bool bRet = false;
    ScDefaultsOptions aOpt;

    SCTAB nTabCount = static_cast<SCTAB>(m_xEdNSheets->get_value());
    OUString aSheetPrefix = m_xEdSheetPrefix->get_text();

    if ( m_xEdNSheets->get_value_changed_from_saved()
         || m_xEdSheetPrefix->get_saved_value() != aSheetPrefix )
    {
        aOpt.SetInitTabCount( nTabCount );
        aOpt.SetInitTabPrefix( aSheetPrefix );

        rCoreSet->Put( ScTpDefaultsItem( aOpt ) );
        bRet = true;
    }
    return bRet;
}

void ScTpDefaultsOptions::Reset(const SfxItemSet* rCoreSet)
{
    ScDefaultsOptions aOpt;
    const SfxPoolItem* pItem = nullptr;

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SCDEFAULTSOPTIONS, false , &pItem))
        aOpt = static_cast<const ScTpDefaultsItem*>(pItem)->GetDefaultsOptions();

    m_xEdNSheets->set_value(aOpt.GetInitTabCount());
    m_xEdSheetPrefix->set_text( aOpt.GetInitTabPrefix() );
    m_xEdNSheets->save_value();
    m_xEdSheetPrefix->save_value();
}

DeactivateRC ScTpDefaultsOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    return DeactivateRC::KeepPage;
}

void ScTpDefaultsOptions::CheckNumSheets()
{
    auto nVal = m_xEdNSheets->get_value();
    if (nVal > MAXINITTAB)
        m_xEdNSheets->set_value(MAXINITTAB);
    if (nVal < MININITTAB)
        m_xEdNSheets->set_value(MININITTAB);
}

void ScTpDefaultsOptions::CheckPrefix()
{
    OUString aSheetPrefix = m_xEdSheetPrefix->get_text();

    if (!aSheetPrefix.isEmpty() && !ScDocument::ValidTabName(aSheetPrefix))
    {
        // Revert to last good Prefix and also select it to
        // indicate something illegal was typed
        m_xEdSheetPrefix->set_text(maOldPrefixValue);
        m_xEdSheetPrefix->select_region(0, -1);
    }
    else
    {
        OnFocusPrefixInput();
    }
}

void ScTpDefaultsOptions::OnFocusPrefixInput()
{
    // Store Prefix in case we need to revert
    maOldPrefixValue = m_xEdSheetPrefix->get_text();
}

IMPL_LINK_NOARG(ScTpDefaultsOptions, NumModifiedHdl, weld::Entry&, void)
{
    CheckNumSheets();
}

IMPL_LINK_NOARG(ScTpDefaultsOptions, PrefixModifiedHdl, weld::Entry&, void)
{
    CheckPrefix();
}

IMPL_LINK_NOARG(ScTpDefaultsOptions, PrefixEditOnFocusHdl, weld::Widget&, void)
{
    OnFocusPrefixInput();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
