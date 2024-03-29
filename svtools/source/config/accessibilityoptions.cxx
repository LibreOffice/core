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


#include <svtools/accessibilityoptions.hxx>

#include <unotools/configmgr.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <officecfg/Office/Common.hxx>

#include <mutex>

#define HELP_TIP_TIMEOUT 0xffff     // max. timeout setting to pretend a non-timeout

namespace SvtAccessibilityOptions
{

void SetVCLSettings()
{
    AllSettings aAllSettings(Application::GetSettings());
    StyleSettings aStyleSettings(aAllSettings.GetStyleSettings());
    HelpSettings aHelpSettings(aAllSettings.GetHelpSettings());
    bool StyleSettingsChanged(false);

    bool bHelpTipsDisappear = officecfg::Office::Common::Accessibility::IsHelpTipsDisappear::get();
    sal_Int16 nHelpTipSeconds = officecfg::Office::Common::Accessibility::HelpTipSeconds::get();
    aHelpSettings.SetTipTimeout( bHelpTipsDisappear ? nHelpTipSeconds * 1000 : HELP_TIP_TIMEOUT);
    aAllSettings.SetHelpSettings(aHelpSettings);

    std::optional<sal_Int16> nEdgeBlendingCount(officecfg::Office::Common::Accessibility::EdgeBlending::get());
    if (!nEdgeBlendingCount)
        nEdgeBlendingCount = 35;
    OSL_ENSURE(*nEdgeBlendingCount >= 0, "OOps, negative values for EdgeBlending are not allowed (!)");
    if (*nEdgeBlendingCount < 0)
        nEdgeBlendingCount = 0;

    if(aStyleSettings.GetEdgeBlending() != *nEdgeBlendingCount)
    {
        aStyleSettings.SetEdgeBlending(*nEdgeBlendingCount);
        StyleSettingsChanged = true;
    }

    std::optional<sal_Int16> nMaxLineCount(officecfg::Office::Common::Accessibility::ListBoxMaximumLineCount::get());
    if (!nMaxLineCount)
        nMaxLineCount = 25;
    OSL_ENSURE(*nMaxLineCount >= 0, "OOps, negative values for ListBoxMaximumLineCount are not allowed (!)");
    if (*nMaxLineCount < 0)
        nMaxLineCount = 0;

    if(aStyleSettings.GetListBoxMaximumLineCount() != *nMaxLineCount)
    {
        aStyleSettings.SetListBoxMaximumLineCount(*nMaxLineCount);
        StyleSettingsChanged = true;
    }

#ifdef IOS
    std::optional<sal_Int16> nMaxColumnCount = 4;
#else
    std::optional<sal_Int16> nMaxColumnCount(officecfg::Office::Common::Accessibility::ColorValueSetColumnCount::get());
    if (!nMaxColumnCount)
        nMaxColumnCount = 12;
#endif

    OSL_ENSURE(*nMaxColumnCount >= 0, "OOps, negative values for ColorValueSetColumnCount are not allowed (!)");
    if (*nMaxColumnCount < 0)
        nMaxColumnCount = 0;

    if(aStyleSettings.GetColorValueSetColumnCount() != *nMaxColumnCount)
    {
        aStyleSettings.SetColorValueSetColumnCount(*nMaxColumnCount);
        StyleSettingsChanged = true;
    }

    std::optional<bool> bTmp = officecfg::Office::Common::Accessibility::PreviewUsesCheckeredBackground::get();
    const bool bPreviewUsesCheckeredBackground = bTmp.value_or(false);

    if(aStyleSettings.GetPreviewUsesCheckeredBackground() != bPreviewUsesCheckeredBackground)
    {
        aStyleSettings.SetPreviewUsesCheckeredBackground(bPreviewUsesCheckeredBackground);
        StyleSettingsChanged = true;
    }

    if(StyleSettingsChanged)
    {
        aAllSettings.SetStyleSettings(aStyleSettings);
        Application::MergeSystemSettings(aAllSettings);
    }

    Application::SetSettings(aAllSettings);
}

} // namespace SvtAccessibilityOptions

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
