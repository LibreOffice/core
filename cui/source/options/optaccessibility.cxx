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

#include "optaccessibility.hxx"
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <officecfg/Office/Common.hxx>

SvxAccessibilityOptionsTabPage::SvxAccessibilityOptionsTabPage(weld::Container* pPage, weld::DialogController* pController,
    const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/optaccessibilitypage.ui"_ustr, u"OptAccessibilityPage"_ustr, &rSet)
    , m_xAccessibilityTool(m_xBuilder->weld_check_button(u"acctool"_ustr))
    , m_xTextSelectionInReadonly(m_xBuilder->weld_check_button(u"textselinreadonly"_ustr))
    , m_xTextSelectionInReadonlyImg(m_xBuilder->weld_widget(u"locktextselinreadonly"_ustr))
    , m_xAnimatedGraphics(m_xBuilder->weld_check_button(u"animatedgraphics"_ustr))
    , m_xAnimatedGraphicsImg(m_xBuilder->weld_widget(u"lockanimatedgraphics"_ustr))
    , m_xAnimatedTexts(m_xBuilder->weld_check_button(u"animatedtext"_ustr))
    , m_xAnimatedTextsImg(m_xBuilder->weld_widget(u"lockanimatedtext"_ustr))
    , m_xHighContrast(m_xBuilder->weld_combo_box(u"highcontrast"_ustr))
    , m_xHighContrastImg(m_xBuilder->weld_widget(u"lockhighcontrast"_ustr))
    , m_xHighContrastLabel(m_xBuilder->weld_label(u"label13"_ustr))
    , m_xAutomaticFontColor(m_xBuilder->weld_check_button(u"autofontcolor"_ustr))
    , m_xAutomaticFontColorImg(m_xBuilder->weld_widget(u"lockautofontcolor"_ustr))
    , m_xPagePreviews(m_xBuilder->weld_check_button(u"systempagepreviewcolor"_ustr))
    , m_xPagePreviewsImg(m_xBuilder->weld_widget(u"locksystempagepreviewcolor"_ustr))
{
#ifdef UNX
    // UNIX: read the gconf2 setting instead to use the checkbox
    m_xAccessibilityTool->hide();
#endif
}

SvxAccessibilityOptionsTabPage::~SvxAccessibilityOptionsTabPage()
{
}

std::unique_ptr<SfxTabPage> SvxAccessibilityOptionsTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SvxAccessibilityOptionsTabPage>(pPage, pController, *rAttrSet);
}

OUString SvxAccessibilityOptionsTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"label2"_ustr, u"label13"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"acctool"_ustr,      u"textselinreadonly"_ustr, u"animatedgraphics"_ustr,
                               u"animatedtext"_ustr, u"autofontcolor"_ustr,     u"systempagepreviewcolor"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SvxAccessibilityOptionsTabPage::FillItemSet( SfxItemSet* )
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch( comphelper::ConfigurationChanges::create() );
    if ( !officecfg::Office::Common::Accessibility::IsForPagePreviews::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsForPagePreviews::set(m_xPagePreviews->get_active(), batch);
    if ( !officecfg::Office::Common::Accessibility::IsAllowAnimatedGraphics::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsAllowAnimatedGraphics::set(m_xAnimatedGraphics->get_active(), batch);
    if ( !officecfg::Office::Common::Accessibility::IsAllowAnimatedText::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsAllowAnimatedText::set(m_xAnimatedTexts->get_active(), batch);
    if ( !officecfg::Office::Common::Accessibility::IsAutomaticFontColor::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsAutomaticFontColor::set(m_xAutomaticFontColor->get_active(), batch);
    if ( !officecfg::Office::Common::Accessibility::IsSelectionInReadonly::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsSelectionInReadonly::set(m_xTextSelectionInReadonly->get_active(), batch);
    if ( !officecfg::Office::Common::Accessibility::HighContrast::isReadOnly() )
        officecfg::Office::Common::Accessibility::HighContrast::set(m_xHighContrast->get_active(), batch);
    batch->commit();

    AllSettings aAllSettings = Application::GetSettings();
    MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
#ifndef UNX
    aMiscSettings.SetEnableATToolSupport(m_xAccessibilityTool->get_active());
#endif
    aAllSettings.SetMiscSettings(aMiscSettings);
    Application::MergeSystemSettings( aAllSettings );
    Application::SetSettings(aAllSettings);

    return false;
}

void SvxAccessibilityOptionsTabPage::Reset( const SfxItemSet* )
{
    m_xPagePreviews->set_active( officecfg::Office::Common::Accessibility::IsForPagePreviews::get() );
    if (officecfg::Office::Common::Accessibility::IsForPagePreviews::isReadOnly())
    {
        m_xPagePreviews->set_sensitive(false);
        m_xPagePreviewsImg->set_visible(true);
    }

    m_xAnimatedGraphics->set_active( officecfg::Office::Common::Accessibility::IsAllowAnimatedGraphics::get() );
    if (officecfg::Office::Common::Accessibility::IsAllowAnimatedGraphics::isReadOnly())
    {
        m_xAnimatedGraphics->set_sensitive(false);
        m_xAnimatedGraphicsImg->set_visible(true);
    }

    m_xAnimatedTexts->set_active( officecfg::Office::Common::Accessibility::IsAllowAnimatedText::get() );
    if (officecfg::Office::Common::Accessibility::IsAllowAnimatedText::isReadOnly())
    {
        m_xAnimatedTexts->set_sensitive(false);
        m_xAnimatedTextsImg->set_visible(true);
    }

    m_xAutomaticFontColor->set_active( officecfg::Office::Common::Accessibility::IsAutomaticFontColor::get() );
    if (officecfg::Office::Common::Accessibility::IsAutomaticFontColor::isReadOnly())
    {
        m_xAutomaticFontColor->set_sensitive(false);
        m_xAutomaticFontColorImg->set_visible(true);
    }

    m_xTextSelectionInReadonly->set_active( officecfg::Office::Common::Accessibility::IsSelectionInReadonly::get() );
    if (officecfg::Office::Common::Accessibility::IsSelectionInReadonly::isReadOnly())
    {
        m_xTextSelectionInReadonly->set_sensitive(false);
        m_xTextSelectionInReadonlyImg->set_visible(true);
    }

    m_xHighContrast->set_active( officecfg::Office::Common::Accessibility::HighContrast::get() );
    if (officecfg::Office::Common::Accessibility::HighContrast::isReadOnly())
    {
        m_xHighContrast->set_sensitive(false);
        m_xHighContrastLabel->set_sensitive(false);
        m_xHighContrastImg->set_visible(true);
    }

    AllSettings aAllSettings = Application::GetSettings();
    const MiscSettings& aMiscSettings = aAllSettings.GetMiscSettings();
    m_xAccessibilityTool->set_active(aMiscSettings.GetEnableATToolSupport());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
