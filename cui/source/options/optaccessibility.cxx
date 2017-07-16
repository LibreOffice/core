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

#include <optaccessibility.hxx>
#include <dialmgr.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <officecfg/Office/Common.hxx>

SvxAccessibilityOptionsTabPage::SvxAccessibilityOptionsTabPage(vcl::Window* pParent,
    const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptAccessibilityPage",
        "cui/ui/optaccessibilitypage.ui", &rSet)
{
    get(m_pAccessibilityTool, "acctool");
    get(m_pTextSelectionInReadonly, "textselinreadonly");
    get(m_pAnimatedGraphics, "animatedgraphics");
    get(m_pAnimatedTexts, "animatedtext");

    get(m_pAutoDetectHC, "autodetecthc");
    get(m_pAutomaticFontColor, "autofontcolor");
    get(m_pPagePreviews, "systempagepreviewcolor");

#ifdef UNX
    // UNIX: read the gconf2 setting instead to use the checkbox
    m_pAccessibilityTool->Hide();
#endif
}

SvxAccessibilityOptionsTabPage::~SvxAccessibilityOptionsTabPage()
{
    disposeOnce();
}

void SvxAccessibilityOptionsTabPage::dispose()
{
    m_pAccessibilityTool.clear();
    m_pTextSelectionInReadonly.clear();
    m_pAnimatedGraphics.clear();
    m_pAnimatedTexts.clear();
    m_pAutoDetectHC.clear();
    m_pAutomaticFontColor.clear();
    m_pPagePreviews.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxAccessibilityOptionsTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxAccessibilityOptionsTabPage>::Create(pParent, *rAttrSet);
}

bool SvxAccessibilityOptionsTabPage::FillItemSet( SfxItemSet* )
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch( comphelper::ConfigurationChanges::create() );
    if ( !officecfg::Office::Common::Accessibility::IsForPagePreviews::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsForPagePreviews::set(m_pPagePreviews->IsChecked(), batch);
    if ( !officecfg::Office::Common::Accessibility::IsAllowAnimatedGraphics::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsAllowAnimatedGraphics::set(m_pAnimatedGraphics->IsChecked(), batch);
    if ( !officecfg::Office::Common::Accessibility::IsAllowAnimatedText::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsAllowAnimatedText::set(m_pAnimatedTexts->IsChecked(), batch);
    if ( !officecfg::Office::Common::Accessibility::IsAutomaticFontColor::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsAutomaticFontColor::set(m_pAutomaticFontColor->IsChecked(), batch);
    if ( !officecfg::Office::Common::Accessibility::IsSelectionInReadonly::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsSelectionInReadonly::set(m_pTextSelectionInReadonly->IsChecked(), batch);
    if ( !officecfg::Office::Common::Accessibility::AutoDetectSystemHC::isReadOnly() )
        officecfg::Office::Common::Accessibility::AutoDetectSystemHC::set(m_pAutoDetectHC->IsChecked(), batch);
    batch->commit();

    AllSettings aAllSettings = Application::GetSettings();
    MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
#ifndef UNX
    aMiscSettings.SetEnableATToolSupport(m_pAccessibilityTool->IsChecked());
#endif
    aAllSettings.SetMiscSettings(aMiscSettings);
    Application::MergeSystemSettings( aAllSettings );
    Application::SetSettings(aAllSettings);

    return false;
}

void SvxAccessibilityOptionsTabPage::Reset( const SfxItemSet* )
{
    m_pPagePreviews->Check( officecfg::Office::Common::Accessibility::IsForPagePreviews::get() );
    if( officecfg::Office::Common::Accessibility::IsForPagePreviews::isReadOnly() )
        m_pPagePreviews->Disable();

    m_pAnimatedGraphics->Check( officecfg::Office::Common::Accessibility::IsAllowAnimatedGraphics::get() );
    if( officecfg::Office::Common::Accessibility::IsAllowAnimatedGraphics::isReadOnly() )
        m_pAnimatedGraphics->Disable();

    m_pAnimatedTexts->Check( officecfg::Office::Common::Accessibility::IsAllowAnimatedText::get() );
    if( officecfg::Office::Common::Accessibility::IsAllowAnimatedText::isReadOnly() )
        m_pAnimatedTexts->Disable();

    m_pAutomaticFontColor->Check( officecfg::Office::Common::Accessibility::IsAutomaticFontColor::get() );
    if( officecfg::Office::Common::Accessibility::IsAutomaticFontColor::isReadOnly() )
        m_pAutomaticFontColor->Disable();

    m_pTextSelectionInReadonly->Check( officecfg::Office::Common::Accessibility::IsSelectionInReadonly::get() );
    if( officecfg::Office::Common::Accessibility::IsSelectionInReadonly::isReadOnly() )
        m_pTextSelectionInReadonly->Disable();

    m_pAutoDetectHC->Check( officecfg::Office::Common::Accessibility::AutoDetectSystemHC::get() );
    if( officecfg::Office::Common::Accessibility::AutoDetectSystemHC::isReadOnly() )
        m_pAutoDetectHC->Disable();

    AllSettings aAllSettings = Application::GetSettings();
    MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
    m_pAccessibilityTool->Check(aMiscSettings.GetEnableATToolSupport());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
