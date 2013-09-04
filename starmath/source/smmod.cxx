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

#include <tools/globname.hxx>
#include <vcl/status.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <svl/whiter.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/viewsh.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/svxids.hrc>
#include <vcl/msgbox.hxx>
#include <vcl/virdev.hxx>
#include <unotools/syslocale.hxx>
#include <tools/rtti.hxx>
#include "smmod.hxx"
#include "symbol.hxx"
#include "config.hxx"
#include "dialog.hxx"
#include "edit.hxx"
#include "view.hxx"
#include "starmath.hrc"
#include "svx/modctrl.hxx"

TYPEINIT1( SmModule, SfxModule );

#define SmModule
#include "smslots.hxx"

#include <svx/xmlsecctrl.hxx>



SmResId::SmResId( sal_uInt16 nId )
    : ResId(nId, *SM_MOD()->GetResMgr())
{
}

/////////////////////////////////////////////////////////////////

SmLocalizedSymbolData::SmLocalizedSymbolData() :
    Resource( SmResId(RID_LOCALIZED_NAMES) ),
    aUiSymbolNamesAry       ( SmResId(RID_UI_SYMBOL_NAMES) ),
    aExportSymbolNamesAry   ( SmResId(RID_EXPORT_SYMBOL_NAMES) ),
    aUiSymbolSetNamesAry    ( SmResId(RID_UI_SYMBOLSET_NAMES) ),
    aExportSymbolSetNamesAry( SmResId(RID_EXPORT_SYMBOLSET_NAMES) ),
    p50NamesAry             ( 0 ),
    p60NamesAry             ( 0 ),
    n50NamesLang            ( LANGUAGE_NONE ),
    n60NamesLang            ( LANGUAGE_NONE )
{
    FreeResource();
}


SmLocalizedSymbolData::~SmLocalizedSymbolData()
{
    delete p50NamesAry;
    delete p60NamesAry;
}


const OUString SmLocalizedSymbolData::GetUiSymbolName( const OUString &rExportName ) const
{
    OUString aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolNamesArray();
    sal_uInt32 nCount = rExportNames.Count();
    for (sal_uInt32 i = 0;  i < nCount  &&  aRes.isEmpty();  ++i)
    {
        if (rExportNames.GetString(i).equals(rExportName))
        {
            aRes = rUiNames.GetString(i);
            break;
        }
    }

    return aRes;
}


const OUString SmLocalizedSymbolData::GetExportSymbolName( const OUString &rUiName ) const
{
    OUString aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolNamesArray();
    sal_uInt32 nCount = rUiNames.Count();
    for (sal_uInt32 i = 0;  i < nCount  &&  aRes.isEmpty();  ++i)
    {
        if (rUiNames.GetString(i).equals(rUiName))
        {
            aRes = rExportNames.GetString(i);
            break;
        }
    }

    return aRes;
}


const OUString SmLocalizedSymbolData::GetUiSymbolSetName( const OUString &rExportName ) const
{
    OUString aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolSetNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolSetNamesArray();
    sal_uInt32 nCount = rExportNames.Count();
    for (sal_uInt32 i = 0;  i < nCount  &&  aRes.isEmpty();  ++i)
    {
        if (rExportNames.GetString(i).equals(rExportName))
        {
            aRes = rUiNames.GetString(i);
            break;
        }
    }

    return aRes;
}


const OUString SmLocalizedSymbolData::GetExportSymbolSetName( const OUString &rUiName ) const
{
    OUString aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolSetNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolSetNamesArray();
    sal_uInt32 nCount = rUiNames.Count();
    for (sal_uInt32 i = 0;  i < nCount  &&  aRes.isEmpty();  ++i)
    {
        if (rUiNames.GetString(i).equals(rUiName))
        {
            aRes = rExportNames.GetString(i);
            break;
        }
    }

    return aRes;
}


const ResStringArray* SmLocalizedSymbolData::Get50NamesArray( LanguageType nLang )
{
    if (nLang != n50NamesLang)
    {
        int nRID;
        switch (nLang)
        {
            case LANGUAGE_FRENCH    : nRID = RID_FRENCH_50_NAMES;  break;
            case LANGUAGE_ITALIAN   : nRID = RID_ITALIAN_50_NAMES;  break;
            case LANGUAGE_SWEDISH   : nRID = RID_SWEDISH_50_NAMES;  break;
            case LANGUAGE_SPANISH   : nRID = RID_SPANISH_50_NAMES;  break;
            default                 : nRID = -1;  break;
        }
        delete p50NamesAry;
        p50NamesAry = 0;
        n50NamesLang = nLang;
        if (-1 != nRID)
            p50NamesAry = new SmNamesArray( n50NamesLang, nRID );
    }

    return p50NamesAry ? &p50NamesAry->GetNamesArray() : 0;
}


const ResStringArray* SmLocalizedSymbolData::Get60NamesArray( LanguageType nLang )
{
    if (nLang != n60NamesLang)
    {
        int nRID;
        switch (nLang)
        {
            case LANGUAGE_FRENCH    : nRID = RID_FRENCH_60_NAMES;  break;
            case LANGUAGE_ITALIAN   : nRID = RID_ITALIAN_60_NAMES;  break;
            case LANGUAGE_SWEDISH   : nRID = RID_SWEDISH_60_NAMES;  break;
            case LANGUAGE_SPANISH   : nRID = RID_SPANISH_60_NAMES;  break;
            default                 : nRID = -1;  break;
        }
        delete p60NamesAry;
        p60NamesAry = 0;
        n60NamesLang = nLang;
        if (-1 != nRID)
            p60NamesAry = new SmNamesArray( n60NamesLang, nRID );
    }

    return p60NamesAry ? &p60NamesAry->GetNamesArray() : 0;
}

/////////////////////////////////////////////////////////////////

SFX_IMPL_INTERFACE(SmModule, SfxModule, SmResId(RID_APPLICATION))
{
    SFX_STATUSBAR_REGISTRATION(SmResId(RID_STATUSBAR));
}


SmModule::SmModule(SfxObjectFactory* pObjFact) :
    SfxModule(SfxApplication::CreateResManager("sm"), sal_False, pObjFact, NULL),
    pColorConfig( 0 ),
    pConfig( 0 ),
    pLocSymbolData( 0 ),
    pSysLocale( 0 ),
    pVirtualDev( 0 )
{
    SetName(OUString("StarMath"));

    SvxModifyControl::RegisterControl(SID_DOC_MODIFIED, this);
}


SmModule::~SmModule()
{
    delete pConfig;
    if (pColorConfig)
        pColorConfig->RemoveListener(this);
    delete pColorConfig;
    delete pLocSymbolData;
    delete pSysLocale;
    delete pVirtualDev;
}

void SmModule::_CreateSysLocale() const
{
    SmModule* pThis = (SmModule*)this;
    pThis->pSysLocale = new SvtSysLocale;
}

void SmModule::_CreateVirtualDev() const
{
    SmModule* pThis = (SmModule*)this;
    pThis->pVirtualDev = new VirtualDevice;
    pThis->pVirtualDev->SetReferenceDevice( VirtualDevice::REFDEV_MODE_MSO1 );
}

void SmModule::ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg )
{
    //invalidate all graphic and edit windows
    const TypeId aSmViewTypeId = TYPE(SmViewShell);
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if ((pViewShell->IsA(aSmViewTypeId)))
        {
            SmViewShell *pSmView = (SmViewShell *) pViewShell;
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
    if(!pColorConfig)
    {
        pColorConfig = new svtools::ColorConfig;
        ApplyColorConfigValues( *pColorConfig );
        pColorConfig->AddListener(this);
    }
    return *pColorConfig;
}

void SmModule::ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 )
{
    ApplyColorConfigValues(*pColorConfig);
}

SmConfig * SmModule::GetConfig()
{
    if(!pConfig)
        pConfig = new SmConfig;
    return pConfig;
}

SmSymbolManager & SmModule::GetSymbolManager()
{
    return GetConfig()->GetSymbolManager();
}

SmLocalizedSymbolData & SmModule::GetLocSymbolData() const
{
    if (!pLocSymbolData)
        ((SmModule *) this)->pLocSymbolData = new SmLocalizedSymbolData;
    return *pLocSymbolData;
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

SfxItemSet*  SmModule::CreateItemSet( sal_uInt16 nId )
{
    SfxItemSet*  pRet = 0;
    if(nId == SID_SM_EDITOPTIONS)
    {
        pRet = new SfxItemSet(GetPool(),
                             //TP_SMPRINT
                             SID_PRINTSIZE,         SID_PRINTSIZE,
                             SID_PRINTZOOM,         SID_PRINTZOOM,
                             SID_PRINTTITLE,        SID_PRINTTITLE,
                             SID_PRINTTEXT,         SID_PRINTTEXT,
                             SID_PRINTFRAME,        SID_PRINTFRAME,
                             SID_NO_RIGHT_SPACES,   SID_NO_RIGHT_SPACES,
                             SID_SAVE_ONLY_USED_SYMBOLS, SID_SAVE_ONLY_USED_SYMBOLS,
                             0 );

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
SfxTabPage*  SmModule::CreateTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet )
{
    SfxTabPage*  pRet = 0;
    if(nId == SID_SM_TP_PRINTOPTIONS)
        pRet = SmPrintOptionsTabPage::Create( pParent, rSet );
    return pRet;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
