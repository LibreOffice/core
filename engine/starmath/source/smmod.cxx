/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include <o3tl/string_view.hxx>
#include <sfx2/objface.hxx>
#include <svl/whiter.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/svxids.hrc>
#include <vcl/virdev.hxx>
#include <unotools/syslocale.hxx>
#include <smmod.hxx>
#include <cfgitem.hxx>
#include <dialog.hxx>
#include <view.hxx>
#include <smmod.hrc>
#include <starmath.hrc>
#include <svx/modctrl.hxx>
#include <svtools/colorcfg.hxx>


#define ShellClass_SmModule
#include <smslots.hxx>

OUString SmResId(TranslateId aId) { return Translate::get(aId, SmModule::get()->GetResLocale()); }

OUString SmLocalizedSymbolData::GetUiSymbolName( std::u16string_view rExportName )
{
    auto it = std::find_if(std::begin(RID_UI_SYMBOL_NAMES), std::end(RID_UI_SYMBOL_NAMES),
        [&rExportName](const auto& rUISymbolTranslateId)
        { return o3tl::equalsAscii(rExportName, rUISymbolTranslateId.getId()); });
    if (it != std::end(RID_UI_SYMBOL_NAMES))
    {
        return SmResId(*it);
    }

    return {};
}

OUString SmLocalizedSymbolData::GetExportSymbolName( std::u16string_view rUiName )
{
    auto it = std::find_if(std::begin(RID_UI_SYMBOL_NAMES), std::end(RID_UI_SYMBOL_NAMES),
        [&rUiName](const auto& rUISymbolTranslateId)
        { return rUiName == SmResId(rUISymbolTranslateId); });
    if (it != std::end(RID_UI_SYMBOL_NAMES))
    {
        const char *pKey = it->getId();
        return OUString(pKey, strlen(pKey), RTL_TEXTENCODING_UTF8);
    }

    return {};
}

OUString SmLocalizedSymbolData::GetUiSymbolSetName( std::u16string_view rExportName )
{
    auto it = std::find_if(std::begin(RID_UI_SYMBOLSET_NAMES), std::end(RID_UI_SYMBOLSET_NAMES),
        [&rExportName](const auto& rUISymbolSetTranslateId)
        { return o3tl::equalsAscii(rExportName, rUISymbolSetTranslateId.getId()); });
    if (it != std::end(RID_UI_SYMBOLSET_NAMES))
    {
        return SmResId(*it);
    }

    return {};
}

OUString SmLocalizedSymbolData::GetExportSymbolSetName( std::u16string_view rUiName )
{
    auto it = std::find_if(std::begin(RID_UI_SYMBOLSET_NAMES), std::end(RID_UI_SYMBOLSET_NAMES),
        [&rUiName](const auto& rUISymbolSetTranslateId)
        { return rUiName == SmResId(rUISymbolSetTranslateId); });
    if (it != std::end(RID_UI_SYMBOLSET_NAMES))
    {
        const char *pKey = it->getId();
        return OUString(pKey, strlen(pKey), RTL_TEXTENCODING_UTF8);
    }

    return {};
}

SFX_IMPL_INTERFACE(SmModule, SfxModule)

void SmModule::InitInterface_Impl()
{
    GetStaticInterface()->RegisterStatusBar(StatusBarId::MathStatusBar);
}

SmModule::SmModule(SfxObjectFactory* pObjFact)
    : SfxModule("sm"_ostr, {pObjFact})
{
    SetName(u"StarMath"_ustr);

    SvxModifyControl::RegisterControl(SID_DOC_MODIFIED, this);
}

SmModule::~SmModule()
{
    if (mpColorConfig)
        mpColorConfig->RemoveListener(this);
    mpVirtualDev.disposeAndClear();
}

svtools::ColorConfig & SmModule::GetColorConfig()
{
    if(!mpColorConfig)
    {
        mpColorConfig.reset(new svtools::ColorConfig);
        mpColorConfig->AddListener(this);
    }
    return *mpColorConfig;
}

void SmModule::ConfigurationChanged(utl::ConfigurationBroadcaster* pBrdCst, ConfigurationHints)
{
    if (pBrdCst != mpColorConfig.get())
        return;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        // FIXME: What if pViewShell is for a different document,
        // but OTOH Math is presumably never used through
        // COKit, so maybe an irrelevant concern?
        if (dynamic_cast<const SmViewShell *>(pViewShell) != nullptr)
            pViewShell->GetWindow()->Invalidate();
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

SmMathConfig * SmModule::GetConfig()
{
    if(!mpConfig)
        mpConfig.reset(new SmMathConfig);
    return mpConfig.get();
}

SmSymbolManager & SmModule::GetSymbolManager()
{
    return GetConfig()->GetSymbolManager();
}

const SvtSysLocale& SmModule::GetSysLocale()
{
    if( !moSysLocale )
        moSysLocale.emplace();
    return *moSysLocale;
}

VirtualDevice &SmModule::GetDefaultVirtualDev()
{
    if (!mpVirtualDev)
    {
        mpVirtualDev.reset( VclPtr<VirtualDevice>::Create() );
        mpVirtualDev->SetReferenceDevice( VirtualDevice::RefDevMode::MSO1 );
    }
    return *mpVirtualDev;
}

void SmModule::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);

    for (sal_uInt16 nWh = aIter.FirstWhich(); 0 != nWh; nWh = aIter.NextWhich())
        switch (nWh)
        {
            case SID_CONFIGEVENT :
                rSet.DisableItem(SID_CONFIGEVENT);
                break;
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
