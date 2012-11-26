/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SMMOD_HXX
#define _SMMOD_HXX

#ifndef _SV_RESARY_HXX
#include <tools/resary.hxx>
#endif
#include <svl/lstner.hxx>
#include <svtools/colorcfg.hxx>

#include <tools/shl.hxx>
#include <sfx2/module.hxx>

#ifndef _STARMATH_HRC
#include "starmath.hrc"
#endif

#include <unotools/options.hxx>

class SvxErrorHandler;
class SfxObjectFactory;
class SmConfig;
class SmModule;
class SmSymbolManager;

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

class SvtSysLocale;
class VirtualDevice;

/////////////////////////////////////////////////////////////////

class SmResId : public ResId
{
public:
    SmResId(sal_uInt16 nId);
};

class SmNamesArray : public Resource
{
    ResStringArray      aNamesAry;
    LanguageType        nLanguage;

public:
    SmNamesArray( LanguageType nLang, int nRID ) :
        Resource( SmResId(RID_LOCALIZED_NAMES) ),
        aNamesAry   (SmResId( static_cast < sal_uInt16 > ( nRID ))),
        nLanguage   (nLang)
    {
        FreeResource();
    }

    LanguageType            GetLanguage() const     { return nLanguage; }
    const ResStringArray&   GetNamesArray() const   { return aNamesAry; }
};

/////////////////////////////////////////////////////////////////

class SmLocalizedSymbolData : public Resource
{
    ResStringArray      aUiSymbolNamesAry;
    ResStringArray      aExportSymbolNamesAry;
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
    const String          GetExportSymbolSetName( const String &rUiName ) const;

    const ResStringArray* Get50NamesArray( LanguageType nLang );
    const ResStringArray* Get60NamesArray( LanguageType nLang );
};

/////////////////////////////////////////////////////////////////

class SmModule : public SfxModule, utl::ConfigurationListener
{
    svtools::ColorConfig        *pColorConfig;
    SmConfig                *pConfig;
    SmLocalizedSymbolData   *pLocSymbolData;
    SvtSysLocale            *pSysLocale;
    VirtualDevice           *pVirtualDev;

    void _CreateSysLocale() const;
    void _CreateVirtualDev() const;

    void ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg );

public:
    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START + 0)

    SmModule(SfxObjectFactory* pObjFact);
    virtual ~SmModule();

    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

    svtools::ColorConfig &  GetColorConfig();

    SmConfig *              GetConfig();
    SmSymbolManager &       GetSymbolManager();

    SmLocalizedSymbolData &   GetLocSymbolData() const;

    void GetState(SfxItemSet&);

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
    virtual SfxItemSet*  CreateItemSet( sal_uInt16 nId );
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );
    virtual SfxTabPage*  CreateTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet );
};

#define SM_MOD() ( *(SmModule**) GetAppData(SHL_SM) )

#endif                                 // _SDMOD_HXX

