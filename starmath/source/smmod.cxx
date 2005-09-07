/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: smmod.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:12:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#pragma hdrstop


#ifndef _GLOBNAME_HXX //autogen
#include <tools/globname.hxx>
#endif
#ifndef _SV_STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _SFXMSG_HXX //autogen
#include <sfx2/msg.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_HRC //autogen
#include <sfx2/sfx.hrc>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _SMMOD_HXX
#include "smmod.hxx"
#endif
#ifndef SYMBOL_HXX
#include "symbol.hxx"
#endif
#ifndef _CONFIG_HXX
#include "config.hxx"
#endif
#ifndef _DIALOG_HXX
#include "dialog.hxx"
#endif
#ifndef EDIT_HXX
#include "edit.hxx"
#endif
#ifndef VIEW_HXX
#include "view.hxx"
#endif
#include "starmath.hrc"

TYPEINIT1( SmModule, SfxModule );

#define SmModule
#include "smslots.hxx"

#include <svx/xmlsecctrl.hxx>



SmResId::SmResId( USHORT nId )
    : ResId(nId, SM_MOD()->GetResMgr())
{
}

/////////////////////////////////////////////////////////////////

SmLocalizedSymbolData::SmLocalizedSymbolData() :
    Resource( SmResId(RID_LOCALIZED_NAMES) ),
    aUiSymbolNamesAry       ( ResId(RID_UI_SYMBOL_NAMES) ),
    aExportSymbolNamesAry   ( ResId(RID_EXPORT_SYMBOL_NAMES) ),
    aUiSymbolSetNamesAry    ( ResId(RID_UI_SYMBOLSET_NAMES) ),
    aExportSymbolSetNamesAry( ResId(RID_EXPORT_SYMBOLSET_NAMES) ),
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

    const SmLocalizedSymbolData &rData = SM_MOD1()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolNamesArray();
    USHORT nCount = rExportNames.Count();
    for (USHORT i = 0;  i < nCount  &&  !aRes.Len();  ++i)
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

    const SmLocalizedSymbolData &rData = SM_MOD1()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolNamesArray();
    USHORT nCount = rUiNames.Count();
    for (USHORT i = 0;  i < nCount  &&  !aRes.Len();  ++i)
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

    const SmLocalizedSymbolData &rData = SM_MOD1()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolSetNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolSetNamesArray();
    USHORT nCount = rExportNames.Count();
    for (USHORT i = 0;  i < nCount  &&  !aRes.Len();  ++i)
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

    const SmLocalizedSymbolData &rData = SM_MOD1()->GetLocSymbolData();
    const ResStringArray &rUiNames = rData.GetUiSymbolSetNamesArray();
    const ResStringArray &rExportNames = rData.GetExportSymbolSetNamesArray();
    USHORT nCount = rUiNames.Count();
    for (USHORT i = 0;  i < nCount  &&  !aRes.Len();  ++i)
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
    SfxModule(SfxApplication::CreateResManager("sm"), FALSE, pObjFact, NULL),
    pConfig( 0 ),
    pColorConfig( 0 ),
    pLocSymbolData( 0 ),
    pRectCache( new SmRectCache ),
    pSysLocale( 0 ),
    pVirtualDev( 0 )
{
    SetName( C2S("StarMath" ));
}


SmModule::~SmModule()
{
    delete pConfig;
    delete pColorConfig;
    delete pLocSymbolData;
    delete pRectCache;
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
        StartListening( *pColorConfig );
    }
    return *pColorConfig;
}

void SmModule::Notify( SfxBroadcaster &rBC, const SfxHint &rHint )
{
    if (rHint.ISA(SfxSimpleHint))
    {
        ULONG nHintId = ((SfxSimpleHint&)rHint).GetId();
        if (SFX_HINT_COLORS_CHANGED == nHintId)
            ApplyColorConfigValues(*pColorConfig);
    }
}

SmConfig * SmModule::GetConfig()
{
    if(!pConfig)
        pConfig = new SmConfig;
    return pConfig;
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

    for (USHORT nWh = aIter.FirstWhich(); 0 != nWh; nWh = aIter.NextWhich())
        switch (nWh)
        {
            case SID_CONFIGEVENT :
                rSet.DisableItem(SID_CONFIGEVENT);
                break;
        }
}

void SmModule::FillStatusBar(StatusBar &rBar)
{
    rBar.InsertItem(SID_TEXTSTATUS, 300, SIB_LEFT | SIB_IN);
    rBar.InsertItem(SID_ATTR_ZOOM, rBar.GetTextWidth(C2S(" 100% ")));
    rBar.InsertItem(SID_MODIFYSTATUS, rBar.GetTextWidth(C2S(" * ")));
    rBar.InsertItem( SID_SIGNATURE, XmlSecStatusBarControl::GetDefItemWidth( rBar ), SIB_USERDRAW );
    rBar.SetHelpId(SID_SIGNATURE, SID_SIGNATURE);

}

/* -----------------15.02.99 12:45-------------------
 *
 * --------------------------------------------------*/
SfxItemSet*  SmModule::CreateItemSet( USHORT nId )
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
                             0 );

            GetConfig()->ConfigToItemSet(*pRet);
    }
    return pRet;
}
/* -----------------15.02.99 12:45-------------------
 *
 * --------------------------------------------------*/
void SmModule::ApplyItemSet( USHORT nId, const SfxItemSet& rSet )
{
    if(nId == SID_SM_EDITOPTIONS)
    {
        GetConfig()->ItemSetToConfig(rSet);
    }
}
/* -----------------15.02.99 12:45-------------------
 *
 * --------------------------------------------------*/
SfxTabPage*  SmModule::CreateTabPage( USHORT nId, Window* pParent, const SfxItemSet& rSet )
{
    SfxTabPage*  pRet = 0;
    if(nId == SID_SM_TP_PRINTOPTIONS)
        pRet = SmPrintOptionsTabPage::Create( pParent, rSet );
    return pRet;

}
