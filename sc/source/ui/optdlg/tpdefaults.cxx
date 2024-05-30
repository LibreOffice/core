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
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Calc.hxx>
#include <config_features.h>

ScTpDefaultsOptions::ScTpDefaultsOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rCoreSet)
    : SfxTabPage(pPage, pController, u"modules/scalc/ui/optdefaultpage.ui"_ustr, u"OptDefaultPage"_ustr, &rCoreSet)
    , m_xEdNSheets(m_xBuilder->weld_spin_button(u"sheetsnumber"_ustr))
    , m_xEdNSheetsImg(m_xBuilder->weld_widget(u"locksheetsnumber"_ustr))
    , m_xEdSheetPrefix(m_xBuilder->weld_entry(u"sheetprefix"_ustr))
    , m_xEdSheetPrefixImg(m_xBuilder->weld_widget(u"locksheetprefix"_ustr))
    , m_xEdJumboSheets(m_xBuilder->weld_check_button(u"jumbo_sheets"_ustr))
    , m_xEdJumboSheetsImg(m_xBuilder->weld_widget(u"lockjumbo_sheets"_ustr))
{
    m_xEdNSheets->connect_changed( LINK(this, ScTpDefaultsOptions, NumModifiedHdl) );
    m_xEdSheetPrefix->connect_changed( LINK(this, ScTpDefaultsOptions, PrefixModifiedHdl) );
    m_xEdSheetPrefix->connect_focus_in( LINK(this, ScTpDefaultsOptions, PrefixEditOnFocusHdl) );
#if HAVE_FEATURE_JUMBO_SHEETS
    if (!officecfg::Office::Common::Misc::ExperimentalMode::get())
#endif
        m_xEdJumboSheets->hide();
}

ScTpDefaultsOptions::~ScTpDefaultsOptions()
{
}

std::unique_ptr<SfxTabPage> ScTpDefaultsOptions::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rCoreAttrs)
{
    return std::make_unique<ScTpDefaultsOptions>(pPage, pController, *rCoreAttrs);
}

OUString ScTpDefaultsOptions::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"textsheetsnumber"_ustr, u"textsheetprefix"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    sAllStrings += m_xEdJumboSheets->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool ScTpDefaultsOptions::FillItemSet(SfxItemSet *rCoreSet)
{
    bool bRet = false;
    ScDefaultsOptions aOpt;

    SCTAB nTabCount = static_cast<SCTAB>(m_xEdNSheets->get_value());
    OUString aSheetPrefix = m_xEdSheetPrefix->get_text();
    bool bJumboSheets = m_xEdJumboSheets->get_state();

    if ( m_xEdNSheets->get_value_changed_from_saved()
         || m_xEdSheetPrefix->get_saved_value() != aSheetPrefix
         || m_xEdJumboSheets->get_saved_state() != (bJumboSheets ? TRISTATE_TRUE : TRISTATE_FALSE) )
    {
        aOpt.SetInitTabCount( nTabCount );
        aOpt.SetInitTabPrefix( aSheetPrefix );
#if HAVE_FEATURE_JUMBO_SHEETS
        aOpt.SetInitJumboSheets( bJumboSheets );
#endif
        rCoreSet->Put( ScTpDefaultsItem( std::move(aOpt) ) );
        bRet = true;
    }
    return bRet;
}

void ScTpDefaultsOptions::Reset(const SfxItemSet* rCoreSet)
{
    ScDefaultsOptions aOpt;

    if(const ScTpDefaultsItem* pDefaultsItem = rCoreSet->GetItemIfSet(SID_SCDEFAULTSOPTIONS, false))
        aOpt = pDefaultsItem->GetDefaultsOptions();

    m_xEdNSheets->set_value(aOpt.GetInitTabCount());
    m_xEdSheetPrefix->set_text( aOpt.GetInitTabPrefix() );
    m_xEdJumboSheets->set_state( aOpt.GetInitJumboSheets() ? TRISTATE_TRUE : TRISTATE_FALSE );

    bool bReadOnly = officecfg::Office::Calc::Defaults::Sheet::SheetCount::isReadOnly();
    m_xEdNSheets->set_sensitive(!bReadOnly);
    m_xEdNSheetsImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Defaults::Sheet::SheetPrefix::isReadOnly();
    m_xEdSheetPrefix->set_sensitive(!bReadOnly);
    m_xEdSheetPrefixImg->set_visible(bReadOnly);

    if (m_xEdJumboSheets->is_visible())
    {
        bReadOnly = officecfg::Office::Calc::Defaults::Sheet::JumboSheets::isReadOnly();
        m_xEdJumboSheets->set_sensitive(!bReadOnly);
        m_xEdJumboSheetsImg->set_visible(bReadOnly);
    }

    m_xEdNSheets->save_value();
    m_xEdSheetPrefix->save_value();
    m_xEdJumboSheets->save_state();
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
