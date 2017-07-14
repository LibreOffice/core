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

#include <o3tl/make_unique.hxx>
#include <sfx2/objface.hxx>
#include <svl/whiter.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/viewsh.hxx>
#include <svx/svxids.hrc>
#include <vcl/virdev.hxx>
#include <unotools/syslocale.hxx>
#include "smmod.hxx"
#include "symbol.hxx"
#include "cfgitem.hxx"
#include "dialog.hxx"
#include "edit.hxx"
#include "view.hxx"
#include "starmath.hrc"
#include "svx/modctrl.hxx"


#define SmModule
#include "smslots.hxx"

OUString SmResId(sal_uInt16 nId)
{
    return ResId(nId, *SM_MOD()->GetResMgr());
}

SmLocalizedSymbolData::SmLocalizedSymbolData() :
    aUiSymbolNamesAry       (ResId(RID_UI_SYMBOL_NAMES, *SM_MOD()->GetResMgr())),
    aExportSymbolNamesAry   (ResId(RID_EXPORT_SYMBOL_NAMES, *SM_MOD()->GetResMgr())),
    aUiSymbolSetNamesAry    (ResId(RID_UI_SYMBOLSET_NAMES, *SM_MOD()->GetResMgr())),
    aExportSymbolSetNamesAry(ResId(RID_EXPORT_SYMBOLSET_NAMES, *SM_MOD()->GetResMgr()))
{
}

SmLocalizedSymbolData::~SmLocalizedSymbolData()
{
}

const OUString SmLocalizedSymbolData::GetUiSymbolName( const OUString &rExportName )
{
    OUString aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolNamesArray();
    sal_uInt32 nCount = rExportNames.Count();
    for (sal_uInt32 i = 0;  i < nCount;  ++i)
    {
        if (rExportNames.GetString(i) == rExportName)
        {
            aRes = rUiNames.GetString(i);
            break;
        }
    }

    return aRes;
}

const OUString SmLocalizedSymbolData::GetExportSymbolName( const OUString &rUiName )
{
    OUString aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolNamesArray();
    sal_uInt32 nCount = rUiNames.Count();
    for (sal_uInt32 i = 0;  i < nCount;  ++i)
    {
        if (rUiNames.GetString(i) == rUiName)
        {
            aRes = rExportNames.GetString(i);
            break;
        }
    }

    return aRes;
}

const OUString SmLocalizedSymbolData::GetUiSymbolSetName( const OUString &rExportName )
{
    OUString aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolSetNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolSetNamesArray();
    sal_uInt32 nCount = rExportNames.Count();
    for (sal_uInt32 i = 0;  i < nCount;  ++i)
    {
        if (rExportNames.GetString(i) == rExportName)
        {
            aRes = rUiNames.GetString(i);
            break;
        }
    }

    return aRes;
}

const OUString SmLocalizedSymbolData::GetExportSymbolSetName( const OUString &rUiName )
{
    OUString aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolSetNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolSetNamesArray();
    sal_uInt32 nCount = rUiNames.Count();
    for (sal_uInt32 i = 0;  i < nCount;  ++i)
    {
        if (rUiNames.GetString(i) == rUiName)
        {
            aRes = rExportNames.GetString(i);
            break;
        }
    }

    return aRes;
}

SFX_IMPL_INTERFACE(SmModule, SfxModule)

void SmModule::InitInterface_Impl()
{
    GetStaticInterface()->RegisterStatusBar(RID_STATUSBAR);
}

SmModule::SmModule(SfxObjectFactory* pObjFact) :
    SfxModule(ResMgr::CreateResMgr("sm"), {pObjFact})
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

SmLocalizedSymbolData & SmModule::GetLocSymbolData()
{
    if (!mpLocSymbolData)
        mpLocSymbolData.reset(new SmLocalizedSymbolData);
    return *mpLocSymbolData;
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
        pRet = o3tl::make_unique<SfxItemSet>(
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

VclPtr<SfxTabPage> SmModule::CreateTabPage( sal_uInt16 nId, vcl::Window* pParent, const SfxItemSet& rSet )
{
    VclPtr<SfxTabPage> pRet;
    if(nId == SID_SM_TP_PRINTOPTIONS)
        pRet = SmPrintOptionsTabPage::Create( pParent, rSet );
    return pRet;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
