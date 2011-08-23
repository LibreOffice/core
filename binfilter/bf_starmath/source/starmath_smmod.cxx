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

#ifndef _SFXAPP_HXX //autogen
#include <bf_sfx2/app.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <bf_svtools/whiter.hxx>
#endif
#ifndef _SFX_HRC //autogen
#include <bf_sfx2/sfx.hrc>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <bf_svx/svxids.hrc>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <bf_svtools/syslocale.hxx>
#endif

#include <vcl/virdev.hxx>

#ifndef _SFXITEMSET_HXX
#include <bf_svtools/itemset.hxx>
#endif

#include "smmod.hxx"
#include <bf_so3/factory.hxx>

#ifndef _CONFIG_HXX
#include "config.hxx"
#endif
#include "starmath.hrc"
namespace binfilter {//STRIP009
/*N*/ TYPEINIT1( SmModuleDummy, SfxModule );
/*N*/ TYPEINIT1( SmModule, SmModuleDummy );

/*N*/ SmLocalizedSymbolData::SmLocalizedSymbolData() :
/*N*/     Resource( SmResId(RID_LOCALIZED_NAMES) ),
/*N*/ 	aUiSymbolNamesAry		( SmResId(RID_UI_SYMBOL_NAMES) ),
/*N*/     aExportSymbolNamesAry   ( SmResId(RID_EXPORT_SYMBOL_NAMES) ),
/*N*/     aUiSymbolSetNamesAry    ( SmResId(RID_UI_SYMBOLSET_NAMES) ),
/*N*/     aExportSymbolSetNamesAry( SmResId(RID_EXPORT_SYMBOLSET_NAMES) ),
/*N*/     p50NamesAry             ( 0 ),
/*N*/     p60NamesAry             ( 0 ),
/*N*/     n50NamesLang            ( LANGUAGE_NONE ),
/*N*/     n60NamesLang            ( LANGUAGE_NONE )
/*N*/ {
/*N*/ 	FreeResource();
/*N*/ }


/*N*/ SmLocalizedSymbolData::~SmLocalizedSymbolData()
/*N*/ {
/*N*/     delete p50NamesAry;
/*N*/     delete p60NamesAry;
/*N*/ }


/*N*/ const String SmLocalizedSymbolData::GetUiSymbolName( const String &rExportName ) const
/*N*/ {
/*N*/ 	String aRes;
/*N*/ 	
/*N*/     const SmLocalizedSymbolData &rData = SM_MOD1()->GetLocSymbolData();
/*N*/     const ResStringArray &rUiNames = rData.GetUiSymbolNamesArray();
/*N*/     const ResStringArray &rExportNames = rData.GetExportSymbolNamesArray();
/*N*/     USHORT nCount = rExportNames.Count();
/*N*/     for (USHORT i = 0;  i < nCount  &&  !aRes.Len();  ++i)
/*N*/ 	{
/*N*/         if (rExportName == rExportNames.GetString(i))
/*N*/ 		{
/*N*/             aRes = rUiNames.GetString(i);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return aRes;
/*N*/ }


/*N*/ const String SmLocalizedSymbolData::GetExportSymbolName( const String &rUiName ) const
/*N*/ {
/*N*/ 	String aRes;
/*N*/ 	
/*N*/     const SmLocalizedSymbolData &rData = SM_MOD1()->GetLocSymbolData();
/*N*/     const ResStringArray &rUiNames = rData.GetUiSymbolNamesArray();
/*N*/     const ResStringArray &rExportNames = rData.GetExportSymbolNamesArray();
/*N*/ 	USHORT nCount = rUiNames.Count();
/*N*/     for (USHORT i = 0;  i < nCount  &&  !aRes.Len();  ++i)
/*N*/ 	{
/*N*/         if (rUiName == rUiNames.GetString(i))
/*N*/ 		{
/*N*/ 			aRes = rExportNames.GetString(i);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return aRes;
/*N*/ }


/*N*/ const String SmLocalizedSymbolData::GetUiSymbolSetName( const String &rExportName ) const
/*N*/ {
/*N*/ 	String aRes;
/*N*/ 	
/*N*/     const SmLocalizedSymbolData &rData = SM_MOD1()->GetLocSymbolData();
/*N*/     const ResStringArray &rUiNames = rData.GetUiSymbolSetNamesArray();
/*N*/     const ResStringArray &rExportNames = rData.GetExportSymbolSetNamesArray();
/*N*/     USHORT nCount = rExportNames.Count();
/*N*/     for (USHORT i = 0;  i < nCount  &&  !aRes.Len();  ++i)
/*N*/ 	{
/*N*/         if (rExportName == rExportNames.GetString(i))
/*N*/ 		{
/*N*/             aRes = rUiNames.GetString(i);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return aRes;
/*N*/ }




/*N*/ const ResStringArray* SmLocalizedSymbolData::Get50NamesArray( LanguageType nLang )
/*N*/ {
/*N*/     if (nLang != n50NamesLang)
/*N*/     {
/*N*/         int nRID;
/*N*/         switch (nLang)
/*N*/         {
/*?*/             case LANGUAGE_FRENCH    : nRID = RID_FRENCH_50_NAMES;  break;
/*?*/             case LANGUAGE_ITALIAN   : nRID = RID_ITALIAN_50_NAMES;  break;
/*?*/             case LANGUAGE_SWEDISH   : nRID = RID_SWEDISH_50_NAMES;  break;
/*?*/             case LANGUAGE_SPANISH   : nRID = RID_SPANISH_50_NAMES;  break;
/*N*/             default                 : nRID = -1;  break;
/*N*/         }
/*N*/         delete p50NamesAry;
/*N*/         p50NamesAry = 0;
/*N*/         n50NamesLang = nLang;
/*N*/         if (-1 != nRID)
/*?*/             p50NamesAry = new SmNamesArray( n50NamesLang, nRID );
/*N*/     }
/*N*/ 
/*N*/     return p50NamesAry ? &p50NamesAry->GetNamesArray() : 0;
/*N*/ }
    

/*N*/ const ResStringArray* SmLocalizedSymbolData::Get60NamesArray( LanguageType nLang )
/*N*/ {
/*N*/     if (nLang != n60NamesLang)
/*N*/     {
/*N*/         int nRID;
/*N*/         switch (nLang)
/*N*/         {
/*?*/             case LANGUAGE_FRENCH    : nRID = RID_FRENCH_60_NAMES;  break;
/*?*/             case LANGUAGE_ITALIAN   : nRID = RID_ITALIAN_60_NAMES;  break;
/*?*/             case LANGUAGE_SWEDISH   : nRID = RID_SWEDISH_60_NAMES;  break;
/*?*/             case LANGUAGE_SPANISH   : nRID = RID_SPANISH_60_NAMES;  break;
/*N*/             default                 : nRID = -1;  break;
/*N*/         }
/*N*/         delete p60NamesAry;
/*N*/         p60NamesAry = 0;
/*N*/         n60NamesLang = nLang;
/*N*/         if (-1 != nRID)
/*?*/             p60NamesAry = new SmNamesArray( n60NamesLang, nRID );
/*N*/     }
/*N*/ 
/*N*/     return p60NamesAry ? &p60NamesAry->GetNamesArray() : 0;
/*N*/ }

/*N*/ SmModule::SmModule(SvFactory* pObjFact) :
/*N*/ 	SmModuleDummy(SFX_APP()->CreateResManager("bf_sm"), FALSE, pObjFact),	//STRIP005
/*N*/     pConfig( 0 ),
/*N*/     pColorConfig( 0 ),
/*N*/     pLocSymbolData( 0 ),
/*N*/     pRectCache( new SmRectCache ),
/*N*/     pSysLocale( 0 ),
/*N*/     pVirtualDev( 0 )
/*N*/ {
/*N*/ 	SetName( C2S("StarMath" ));
/*N*/ }


/*N*/ SmModule::~SmModule()
/*N*/ {
/*N*/ 	delete pConfig;
/*N*/     delete pColorConfig;
/*N*/     delete pLocSymbolData;
/*N*/ 	delete pRectCache;
/*N*/ 	delete pSysLocale;
/*N*/     delete pVirtualDev;
/*N*/ }

/*N*/ void SmModule::_CreateSysLocale() const
/*N*/ {
/*N*/ 	SmModule* pThis = (SmModule*)this;
/*N*/ 	pThis->pSysLocale = new SvtSysLocale;
/*N*/ }

/*N*/ void SmModule::_CreateVirtualDev() const
/*N*/ {
/*N*/ 	SmModule* pThis = (SmModule*)this;
/*N*/     pThis->pVirtualDev = new VirtualDevice;
/*N*/ }

/*N*/ void SmModule::ApplyColorConfigValues( const ColorConfig &rColorCfg )
/*N*/ {
/*N*/ }

/*N*/ ColorConfig & SmModule::GetColorConfig()
/*N*/ {
/*N*/     if(!pColorConfig)
/*N*/     {
/*N*/         pColorConfig = new ColorConfig;
/*N*/         ApplyColorConfigValues( *pColorConfig );
/*N*/         StartListening( *pColorConfig );
/*N*/     }
/*N*/     return *pColorConfig;
/*N*/ }

/*N*/ void SmModule::Notify( SfxBroadcaster &rBC, const SfxHint &rHint )
/*N*/ {
/*N*/     if (rHint.ISA(SfxSimpleHint))
/*N*/     {
/*N*/         ULONG nHintId = ((SfxSimpleHint&)rHint).GetId();
/*N*/         if (SFX_HINT_COLORS_CHANGED == nHintId)
/*N*/             ApplyColorConfigValues(*pColorConfig);
/*N*/     }
/*N*/ }

/*N*/ SmConfig * SmModule::GetConfig()
/*N*/ {
/*N*/     if(!pConfig)
/*N*/         pConfig = new SmConfig;
/*N*/     return pConfig;
/*N*/ }

/*N*/ SmLocalizedSymbolData & SmModule::GetLocSymbolData() const
/*N*/ {
/*N*/     if (!pLocSymbolData)
/*N*/         ((SmModule *) this)->pLocSymbolData = new SmLocalizedSymbolData;
/*N*/     return *pLocSymbolData;
/*N*/ }

/*N*/ SfxModule *SmModuleDummy::Load()
/*N*/ {
/*N*/ 	return 0;
/*N*/ }

}
