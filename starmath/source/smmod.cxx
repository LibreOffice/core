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

OUString SmResId(TranslateId aId)
{
    return Translate::get(aId, SM_MOD()->GetResLocale());
}

OUString SmLocalizedSymbolData::GetUiSymbolName( std::u16string_view rExportName )
{
    OUString aRes;

    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_UI_SYMBOL_NAMES); ++i)
    {
        if (o3tl::equalsAscii(rExportName, RID_UI_SYMBOL_NAMES[i].mpId))
        {
            aRes = SmResId(RID_UI_SYMBOL_NAMES[i]);
            break;
        }
    }

    return aRes;
}

OUString SmLocalizedSymbolData::GetExportSymbolName( std::u16string_view rUiName )
{
    OUString aRes;

    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_UI_SYMBOL_NAMES); ++i)
    {
        if (rUiName == SmResId(RID_UI_SYMBOL_NAMES[i]))
        {
            const char *pKey = RID_UI_SYMBOL_NAMES[i].mpId;
            aRes = OUString(pKey, strlen(pKey), RTL_TEXTENCODING_UTF8);
            break;
        }
    }

    return aRes;
}

OUString SmLocalizedSymbolData::GetUiSymbolSetName( std::u16string_view rExportName )
{
    OUString aRes;

    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_UI_SYMBOLSET_NAMES); ++i)
    {
        if (o3tl::equalsAscii(rExportName, RID_UI_SYMBOLSET_NAMES[i].mpId))
        {
            aRes = SmResId(RID_UI_SYMBOLSET_NAMES[i]);
            break;
        }
    }

    return aRes;
}

OUString SmLocalizedSymbolData::GetExportSymbolSetName( std::u16string_view rUiName )
{
    OUString aRes;

    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_UI_SYMBOLSET_NAMES); ++i)
    {
        if (rUiName == SmResId(RID_UI_SYMBOLSET_NAMES[i]))
        {
            const char *pKey = RID_UI_SYMBOLSET_NAMES[i].mpId;
            aRes = OUString(pKey, strlen(pKey), RTL_TEXTENCODING_UTF8);
            break;
        }
    }

    return aRes;
}

SFX_IMPL_INTERFACE(SmModule, SfxModule)

void SmModule::InitInterface_Impl()
{
    GetStaticInterface()->RegisterStatusBar(StatusBarId::MathStatusBar);
}

SmModule::SmModule(SfxObjectFactory* pObjFact)
    : SfxModule("sm", {pObjFact})
{
    SetName("StarMath");

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
        // LibreOfficeKit, so maybe an irrelevant concern?
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
    if( !mpSysLocale )
        mpSysLocale.reset(new SvtSysLocale);
    return *mpSysLocale;
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

std::optional<SfxItemSet> SmModule::CreateItemSet( sal_uInt16 nId )
{
    std::optional<SfxItemSet> pRet;
    if(nId == SID_SM_EDITOPTIONS)
    {
        pRet.emplace(
            GetPool(),
            svl::Items< //TP_SMPRINT
                SID_PRINTTITLE, SID_PRINTZOOM,
                SID_NO_RIGHT_SPACES, SID_SAVE_ONLY_USED_SYMBOLS,
                SID_AUTO_CLOSE_BRACKETS, SID_SMEDITWINDOWZOOM>);

        GetConfig()->ConfigToItemSet(*pRet);
    }
    return pRet;
}

void SmModule::ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet )
{
    if(nId == SID_SM_EDITOPTIONS)
    {
        GetConfig()->ItemSetToConfig(rSet);
    }
}

std::unique_ptr<SfxTabPage> SmModule::CreateTabPage( sal_uInt16 nId, weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet )
{
    std::unique_ptr<SfxTabPage> xRet;
    if (nId == SID_SM_TP_PRINTOPTIONS)
        xRet = SmPrintOptionsTabPage::Create(pPage, pController, rSet);
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
