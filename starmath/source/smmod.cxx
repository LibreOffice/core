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

#include <sfx2/objface.hxx>
#include <svl/whiter.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/svxids.hrc>
#include <unotools/resmgr.hxx>
#include <vcl/virdev.hxx>
#include <unotools/syslocale.hxx>
#include <smmod.hxx>
#include "cfgitem.hxx"
#include <dialog.hxx>
#include <edit.hxx>
#include <view.hxx>
#include <smmod.hrc>
#include <starmath.hrc>
#include <svx/modctrl.hxx>
#include <svtools/colorcfg.hxx>


#define ShellClass_SmModule
#include <smslots.hxx>

OUString SmResId(const char* pId)
{
    return Translate::get(pId, SM_MOD()->GetResLocale());
}

const OUString SmLocalizedSymbolData::GetUiSymbolName( const OUString &rExportName )
{
    OUString aRes;

    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_UI_SYMBOL_NAMES); ++i)
    {
        if (rExportName.equalsAscii(strchr(RID_UI_SYMBOL_NAMES[i], '\004') + 1))
        {
            aRes = SmResId(RID_UI_SYMBOL_NAMES[i]);
            break;
        }
    }

    return aRes;
}

const OUString SmLocalizedSymbolData::GetExportSymbolName( const OUString &rUiName )
{
    OUString aRes;

    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_UI_SYMBOL_NAMES); ++i)
    {
        if (rUiName == SmResId(RID_UI_SYMBOL_NAMES[i]))
        {
            const char *pKey = strchr(RID_UI_SYMBOL_NAMES[i], '\004') + 1;
            aRes = OUString(pKey, strlen(pKey), RTL_TEXTENCODING_UTF8);
            break;
        }
    }

    return aRes;
}

const OUString SmLocalizedSymbolData::GetUiSymbolSetName( const OUString &rExportName )
{
    OUString aRes;

    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_UI_SYMBOLSET_NAMES); ++i)
    {
        if (rExportName.equalsAscii(strchr(RID_UI_SYMBOLSET_NAMES[i], '\004') + 1))
        {
            aRes = SmResId(RID_UI_SYMBOLSET_NAMES[i]);
            break;
        }
    }

    return aRes;
}

const OUString SmLocalizedSymbolData::GetExportSymbolSetName( const OUString &rUiName )
{
    OUString aRes;

    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_UI_SYMBOLSET_NAMES); ++i)
    {
        if (rUiName == SmResId(RID_UI_SYMBOLSET_NAMES[i]))
        {
            const char *pKey = strchr(RID_UI_SYMBOLSET_NAMES[i], '\004') + 1;
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

void SmModule::ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg )
{
    //invalidate all graphic and edit windows
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (dynamic_cast<const SmViewShell *>(pViewShell) != nullptr)
        {
            SmViewShell *pSmView = static_cast<SmViewShell *>(pViewShell);
            pSmView->GetGraphicWindow().ApplyColorConfigValues( rColorCfg );
            SmEditWindow *pEditWin = pSmView->GetEditWindow();
            if (pEditWin)
                pEditWin->ApplyColorConfigValues( rColorCfg );
        }
        pViewShell = SfxViewShell::GetNext( *pViewShell );
    }
}

svtools::ColorConfig & SmModule::GetColorConfig()
{
    if(!mpColorConfig)
    {
        mpColorConfig.reset(new svtools::ColorConfig);
        ApplyColorConfigValues( *mpColorConfig );
        mpColorConfig->AddListener(this);
    }
    return *mpColorConfig;
}

void SmModule::ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints )
{
    ApplyColorConfigValues(*mpColorConfig);
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

std::unique_ptr<SfxItemSet> SmModule::CreateItemSet( sal_uInt16 nId )
{
    std::unique_ptr<SfxItemSet> pRet;
    if(nId == SID_SM_EDITOPTIONS)
    {
        pRet = std::make_unique<SfxItemSet>(
            GetPool(),
            svl::Items< //TP_SMPRINT
                SID_PRINTTITLE, SID_PRINTZOOM,
                SID_NO_RIGHT_SPACES, SID_SAVE_ONLY_USED_SYMBOLS,
                SID_AUTO_CLOSE_BRACKETS, SID_AUTO_CLOSE_BRACKETS>{});

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

VclPtr<SfxTabPage> SmModule::CreateTabPage( sal_uInt16 nId, TabPageParent pParent, const SfxItemSet& rSet )
{
    VclPtr<SfxTabPage> pRet;
    if (nId == SID_SM_TP_PRINTOPTIONS)
        pRet = SmPrintOptionsTabPage::Create(pParent, rSet);
    return pRet;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
