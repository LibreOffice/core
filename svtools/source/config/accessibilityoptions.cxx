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
#include <comphelper/processfactory.hxx>

#include <officecfg/Office/Common.hxx>

#include <svl/hint.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <rtl/instance.hxx>
#include <tools/diagnose_ex.h>

#include "itemholder2.hxx"

using namespace utl;
using namespace com::sun::star::uno;

#define HELP_TIP_TIMEOUT 0xffff     // max. timeout setting to pretend a non-timeout

namespace
{
    struct SingletonMutex
        : public rtl::Static< ::osl::Mutex, SingletonMutex > {};
}

// class SvtAccessibilityOptions --------------------------------------------------

SvtAccessibilityOptions::SvtAccessibilityOptions()
{
    if (!utl::ConfigManager::IsFuzzing())
    {
        ::osl::MutexGuard aGuard( SingletonMutex::get() );
        svtools::ItemHolder2::holdConfigItem(EItem::AccessibilityOptions);
    }
}

SvtAccessibilityOptions::~SvtAccessibilityOptions()
{
}

void SvtAccessibilityOptions::Notify( SfxBroadcaster&, const SfxHint&  )
{
    NotifyListeners(ConfigurationHints::NONE);
}


bool SvtAccessibilityOptions::GetIsForPagePreviews() const
{
    return officecfg::Office::Common::Accessibility::IsForPagePreviews::get();
}

bool SvtAccessibilityOptions::GetIsAllowAnimatedGraphics() const
{
    return officecfg::Office::Common::Accessibility::IsAllowAnimatedGraphics::get();
}

bool SvtAccessibilityOptions::GetIsAllowAnimatedText() const
{
    return officecfg::Office::Common::Accessibility::IsAllowAnimatedText::get();
}

bool SvtAccessibilityOptions::GetIsAutomaticFontColor() const
{
    return officecfg::Office::Common::Accessibility::IsAutomaticFontColor::get();
}

bool SvtAccessibilityOptions::IsSelectionInReadonly() const
{
    return officecfg::Office::Common::Accessibility::IsSelectionInReadonly::get();
}

bool SvtAccessibilityOptions::GetIsHelpTipsDisappear() const
{
    return officecfg::Office::Common::Accessibility::IsHelpTipsDisappear::get();
}

sal_Int16 SvtAccessibilityOptions::GetHelpTipSeconds() const
{
    o3tl::optional<sal_Int16> x(officecfg::Office::Common::Accessibility::HelpTipSeconds::get());
    if (!x)
        return 4;
    else
        return *x;
}

sal_Int16 SvtAccessibilityOptions::GetEdgeBlending() const
{
    o3tl::optional<sal_Int16> x(officecfg::Office::Common::Accessibility::EdgeBlending::get());
    if (!x)
        return 35;
    else
        return *x;
}

sal_Int16 SvtAccessibilityOptions::GetListBoxMaximumLineCount() const
{
    o3tl::optional<sal_Int16> x(officecfg::Office::Common::Accessibility::ListBoxMaximumLineCount::get());
    if (!x)
        return 25;
    else
        return *x;
}

sal_Int16 SvtAccessibilityOptions::GetColorValueSetColumnCount() const
{
#ifdef IOS
    return 4;
#else
    o3tl::optional<sal_Int16> x(officecfg::Office::Common::Accessibility::ColorValueSetColumnCount::get());
    if (!x)
        return 12;
    else
        return *x;
#endif
}

bool SvtAccessibilityOptions::GetPreviewUsesCheckeredBackground() const
{
    o3tl::optional<bool> x(officecfg::Office::Common::Accessibility::PreviewUsesCheckeredBackground::get());
    if (!x)
        return false;
    else
        return *x;
}


void SvtAccessibilityOptions::SetVCLSettings()
{
    AllSettings aAllSettings(Application::GetSettings());
    StyleSettings aStyleSettings(aAllSettings.GetStyleSettings());
    HelpSettings aHelpSettings(aAllSettings.GetHelpSettings());
    bool StyleSettingsChanged(false);

    aHelpSettings.SetTipTimeout( GetIsHelpTipsDisappear() ? GetHelpTipSeconds() * 1000 : HELP_TIP_TIMEOUT);
    aAllSettings.SetHelpSettings(aHelpSettings);

    const sal_Int16 nEdgeBlendingCountA(GetEdgeBlending());
    OSL_ENSURE(nEdgeBlendingCountA >= 0, "OOps, negative values for EdgeBlending are not allowed (!)");
    const sal_uInt16 nEdgeBlendingCountB(static_cast< sal_uInt16 >(nEdgeBlendingCountA >= 0 ? nEdgeBlendingCountA : 0));

    if(aStyleSettings.GetEdgeBlending() != nEdgeBlendingCountB)
    {
        aStyleSettings.SetEdgeBlending(nEdgeBlendingCountB);
        StyleSettingsChanged = true;
    }

    const sal_Int16 nMaxLineCountA(GetListBoxMaximumLineCount());
    OSL_ENSURE(nMaxLineCountA >= 0, "OOps, negative values for ListBoxMaximumLineCount are not allowed (!)");
    const sal_uInt16 nMaxLineCountB(static_cast< sal_uInt16 >(nMaxLineCountA >= 0 ? nMaxLineCountA : 0));

    if(aStyleSettings.GetListBoxMaximumLineCount() != nMaxLineCountB)
    {
        aStyleSettings.SetListBoxMaximumLineCount(nMaxLineCountB);
        StyleSettingsChanged = true;
    }

    const sal_Int16 nMaxColumnCountA(GetColorValueSetColumnCount());
    OSL_ENSURE(nMaxColumnCountA >= 0, "OOps, negative values for ColorValueSetColumnCount are not allowed (!)");
    const sal_uInt16 nMaxColumnCountB(static_cast< sal_uInt16 >(nMaxColumnCountA >= 0 ? nMaxColumnCountA : 0));

    if(aStyleSettings.GetColorValueSetColumnCount() != nMaxColumnCountB)
    {
        aStyleSettings.SetColorValueSetColumnCount(nMaxColumnCountB);
        StyleSettingsChanged = true;
    }

    const bool bPreviewUsesCheckeredBackground(GetPreviewUsesCheckeredBackground());

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
