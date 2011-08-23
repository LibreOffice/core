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

#ifndef _SMMOD_HXX
#define _SMMOD_HXX

#ifndef _SV_RESARY_HXX
#include <tools/resary.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <bf_svtools/lstner.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <bf_svtools/colorcfg.hxx>
#endif

#ifndef _SMDLL_HXX
#define _SM_DLL             // fuer SD_MOD()
#include <bf_starmath/smdll.hxx>        // fuer SdModuleDummy
#endif
#ifndef _STARMATH_HRC
#include "starmath.hrc"
#endif
class VirtualDevice; 

namespace binfilter {

class SvxErrorHandler;
class SvtSysLocale; 
class SvFactory; 


class SmConfig;
class SmModule;

/*************************************************************************
|*
|* This subclass of <SfxModule> (which is a subclass of <SfxShell>) is
|* linked to the DLL. One instance of this class exists while the DLL is
|* loaded.
|*
|* SdModule is like to be compared with the <SfxApplication>-subclass.
|*
|* Remember: Don`t export this class! It uses DLL-internal symbols.
|*
\************************************************************************/

class SmRectCache;

/////////////////////////////////////////////////////////////////

/*N*/ class SmNamesArray : public Resource
/*N*/ {
/*N*/     ResStringArray      aNamesAry;
/*N*/     LanguageType        nLanguage;
/*N*/ 
/*N*/ public:
/*N*/     SmNamesArray( LanguageType nLang, int nRID ) :
/*N*/         Resource( SmResId(RID_LOCALIZED_NAMES) ),
/*N*/         nLanguage   (nLang),
/*N*/         aNamesAry   (SmResId(nRID))
/*N*/     {
/*N*/         FreeResource();
/*N*/     }
/*N*/     
/*N*/     LanguageType            GetLanguage() const     { return nLanguage; }
/*N*/     const ResStringArray&   GetNamesArray() const   { return aNamesAry; }
/*N*/ };

/////////////////////////////////////////////////////////////////

class SmLocalizedSymbolData : public Resource
{
    ResStringArray		aUiSymbolNamesAry;
    ResStringArray		aExportSymbolNamesAry;
    ResStringArray      aUiSymbolSetNamesAry;
    ResStringArray      aExportSymbolSetNamesAry;
    SmNamesArray       *p50NamesAry;
    SmNamesArray       *p60NamesAry;
    LanguageType        n50NamesLang;
    LanguageType        n60NamesLang;

public:
    SmLocalizedSymbolData();
    ~SmLocalizedSymbolData();

    const ResStringArray& GetUiSymbolNamesArray() const     { return aUiSymbolNamesAry; }
    const ResStringArray& GetExportSymbolNamesArray() const { return aExportSymbolNamesAry; }
    const String          GetUiSymbolName( const String &rExportName ) const;
    const String          GetExportSymbolName( const String &rUiName ) const;

    const ResStringArray& GetUiSymbolSetNamesArray() const     { return aUiSymbolSetNamesAry; }
    const ResStringArray& GetExportSymbolSetNamesArray() const { return aExportSymbolSetNamesAry; }
    const String          GetUiSymbolSetName( const String &rExportName ) const;

    const ResStringArray* Get50NamesArray( LanguageType nLang );
    const ResStringArray* Get60NamesArray( LanguageType nLang );
};

/////////////////////////////////////////////////////////////////

class SmModule : public SmModuleDummy, public SfxListener
{
    ColorConfig        *pColorConfig;
    SmConfig                *pConfig;
    SmLocalizedSymbolData   *pLocSymbolData;
    SmRectCache             *pRectCache;
    SvtSysLocale            *pSysLocale;
    VirtualDevice           *pVirtualDev;

    void _CreateSysLocale() const;
    void _CreateVirtualDev() const;

    void ApplyColorConfigValues( const ColorConfig &rColorCfg );

public:
    TYPEINFO();

    SmModule(SvFactory* pObjFact);
    virtual ~SmModule();


    // SfxListener
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ColorConfig &  GetColorConfig();

    SmConfig *          GetConfig();
    SmRectCache *		GetRectCache()	   { return pRectCache; }

    SmLocalizedSymbolData &   GetLocSymbolData() const;

    const SvtSysLocale& GetSysLocale() const
    {
        if( !pSysLocale )
            _CreateSysLocale();
        return *pSysLocale;
    }

    VirtualDevice &     GetDefaultVirtualDev()
    {
        if (!pVirtualDev)
            _CreateVirtualDev();
        return *pVirtualDev;
    }

    //virtuelle Methoden fuer den Optionendialog
};

#define SM_MOD1() ( *(SmModule**) GetAppData(BF_SHL_SM) )

} //namespace binfilter
#endif                                 // _SDMOD_HXX

