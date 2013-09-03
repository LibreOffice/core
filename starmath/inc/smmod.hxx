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

#ifndef _SMMOD_HXX
#define _SMMOD_HXX

#include <tools/resary.hxx>
#include <svl/lstner.hxx>
#include <svtools/colorcfg.hxx>

#include <tools/shl.hxx>
#include <sfx2/module.hxx>

#include "starmath.hrc"

#include <unotools/options.hxx>

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

#define SM_RESSTR(x) SmResId(x).toString()

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
    const OUString        GetUiSymbolName( const OUString &rExportName ) const;
    const OUString        GetExportSymbolName( const OUString &rUiName ) const;

    const ResStringArray& GetUiSymbolSetNamesArray() const     { return aUiSymbolSetNamesAry; }
    const ResStringArray& GetExportSymbolSetNamesArray() const { return aExportSymbolSetNamesAry; }
    const OUString        GetUiSymbolSetName( const OUString &rExportName ) const;
    const OUString        GetExportSymbolSetName( const OUString &rUiName ) const;

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
    TYPEINFO();
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

    //virtual methods for options dialog
    virtual SfxItemSet*  CreateItemSet( sal_uInt16 nId );
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );
    virtual SfxTabPage*  CreateTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet );
};

#define SM_MOD() ( *(SmModule**) GetAppData(SHL_SM) )

#endif                                 // _SDMOD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
