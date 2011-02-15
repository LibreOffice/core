/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"


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
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#include <vcl/msgbox.hxx>
#include <vcl/virdev.hxx>
#include <unotools/syslocale.hxx>
#include <tools/rtti.hxx>
#include "smmod.hxx"
#include "symbol.hxx"
#include "config.hxx"
#ifndef _DIALOG_HXX
#include "dialog.hxx"
#endif
#include "edit.hxx"
#include "view.hxx"
#include "starmath.hrc"

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


const String SmLocalizedSymbolData::GetUiSymbolName( const String &rExportName ) const
{
    String aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolNamesArray();
    sal_uInt16 nCount = sal::static_int_cast< xub_StrLen >(rExportNames.Count());
    for (sal_uInt16 i = 0;  i < nCount  &&  !aRes.Len();  ++i)
    {
        if (rExportName == rExportNames.GetString(i))
        {
            aRes = rUiNames.GetString(i);
            break;
        }
    }

    return aRes;
}


const String SmLocalizedSymbolData::GetExportSymbolName( const String &rUiName ) const
{
    String aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolNamesArray();
    sal_uInt16 nCount = sal::static_int_cast< xub_StrLen >(rUiNames.Count());
    for (sal_uInt16 i = 0;  i < nCount  &&  !aRes.Len();  ++i)
    {
        if (rUiName == rUiNames.GetString(i))
        {
            aRes = rExportNames.GetString(i);
            break;
        }
    }

    return aRes;
}


const String SmLocalizedSymbolData::GetUiSymbolSetName( const String &rExportName ) const
{
    String aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolSetNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolSetNamesArray();
    sal_uInt16 nCount = sal::static_int_cast< xub_StrLen >(rExportNames.Count());
    for (sal_uInt16 i = 0;  i < nCount  &&  !aRes.Len();  ++i)
    {
        if (rExportName == rExportNames.GetString(i))
        {
            aRes = rUiNames.GetString(i);
            break;
        }
    }

    return aRes;
}


const String SmLocalizedSymbolData::GetExportSymbolSetName( const String &rUiName ) const
{
    String aRes;

    const SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolSetNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolSetNamesArray();
    sal_uInt16 nCount = sal::static_int_cast< xub_StrLen >(rUiNames.Count());
    for (sal_uInt16 i = 0;  i < nCount  &&  !aRes.Len();  ++i)
    {
        if (rUiName == rUiNames.GetString(i))
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
    SetName( C2S("StarMath" ));
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

/* -----------------15.02.99 12:45-------------------
 *
 * --------------------------------------------------*/
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
/* -----------------15.02.99 12:45-------------------
 *
 * --------------------------------------------------*/
void SmModule::ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet )
{
    if(nId == SID_SM_EDITOPTIONS)
    {
        GetConfig()->ItemSetToConfig(rSet);
    }
}
/* -----------------15.02.99 12:45-------------------
 *
 * --------------------------------------------------*/
SfxTabPage*  SmModule::CreateTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet )
{
    SfxTabPage*  pRet = 0;
    if(nId == SID_SM_TP_PRINTOPTIONS)
        pRet = SmPrintOptionsTabPage::Create( pParent, rSet );
    return pRet;

}
