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
#include <cuires.hrc>
#include <svtools/accessibilityoptions.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

struct SvxAccessibilityOptionsTabPage_Impl
{
    SvtAccessibilityOptions     m_aConfig;
    SvxAccessibilityOptionsTabPage_Impl()
            : m_aConfig(){}
};

SvxAccessibilityOptionsTabPage::SvxAccessibilityOptionsTabPage(vcl::Window* pParent,
    const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptAccessibilityPage",
        "cui/ui/optaccessibilitypage.ui", &rSet)
    , m_pImpl(new SvxAccessibilityOptionsTabPage_Impl)
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
    delete m_pImpl;
    m_pImpl = nullptr;
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
    //aConfig.Set... from controls

    m_pImpl->m_aConfig.SetIsForPagePreviews( m_pPagePreviews->IsChecked() );
    m_pImpl->m_aConfig.SetIsAllowAnimatedGraphics( m_pAnimatedGraphics->IsChecked() );
    m_pImpl->m_aConfig.SetIsAllowAnimatedText( m_pAnimatedTexts->IsChecked() );
    m_pImpl->m_aConfig.SetIsAutomaticFontColor( m_pAutomaticFontColor->IsChecked() );
    m_pImpl->m_aConfig.SetSelectionInReadonly( m_pTextSelectionInReadonly->IsChecked());
    m_pImpl->m_aConfig.SetAutoDetectSystemHC( m_pAutoDetectHC->IsChecked());

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
    //set controls from aConfig.Get...

    m_pPagePreviews->Check(            m_pImpl->m_aConfig.GetIsForPagePreviews() );
    m_pAnimatedGraphics->Check(        m_pImpl->m_aConfig.GetIsAllowAnimatedGraphics() );
    m_pAnimatedTexts->Check(           m_pImpl->m_aConfig.GetIsAllowAnimatedText() );
    m_pAutomaticFontColor->Check(      m_pImpl->m_aConfig.GetIsAutomaticFontColor() );
    m_pTextSelectionInReadonly->Check( m_pImpl->m_aConfig.IsSelectionInReadonly() );
    m_pAutoDetectHC->Check(            m_pImpl->m_aConfig.GetAutoDetectSystemHC() );


    AllSettings aAllSettings = Application::GetSettings();
    MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
    m_pAccessibilityTool->Check(aMiscSettings.GetEnableATToolSupport());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
