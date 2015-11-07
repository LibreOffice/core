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

#ifndef INCLUDED_STARMATH_INC_SMMOD_HXX
#define INCLUDED_STARMATH_INC_SMMOD_HXX

#include <tools/resary.hxx>
#include <svl/lstner.hxx>
#include <svtools/colorcfg.hxx>

#include <tools/shl.hxx>
#include "tools/rc.hxx"
#include <sfx2/module.hxx>

#include "starmath.hrc"

#include <unotools/options.hxx>
#include <memory>

class SfxObjectFactory;
class SmModule;
class SmSymbolManager;
class SmMathConfig;

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



class SmResId : public ResId
{
public:
    SmResId(sal_uInt16 nId);
};

#define SM_RESSTR(x) SmResId(x).toString()

class SmLocalizedSymbolData : public Resource
{
    ResStringArray      aUiSymbolNamesAry;
    ResStringArray      aExportSymbolNamesAry;
    ResStringArray      aUiSymbolSetNamesAry;
    ResStringArray      aExportSymbolSetNamesAry;

public:
    SmLocalizedSymbolData();
    ~SmLocalizedSymbolData();

    const ResStringArray& GetUiSymbolNamesArray() const     { return aUiSymbolNamesAry; }
    const ResStringArray& GetExportSymbolNamesArray() const { return aExportSymbolNamesAry; }
    static const OUString GetUiSymbolName( const OUString &rExportName );
    static const OUString GetExportSymbolName( const OUString &rUiName );

    const ResStringArray& GetUiSymbolSetNamesArray() const     { return aUiSymbolSetNamesAry; }
    const ResStringArray& GetExportSymbolSetNamesArray() const { return aExportSymbolSetNamesAry; }
    static const OUString GetUiSymbolSetName( const OUString &rExportName );
    static const OUString GetExportSymbolSetName( const OUString &rUiName );
};

class SmModule : public SfxModule, utl::ConfigurationListener
{
    std::unique_ptr<svtools::ColorConfig> mpColorConfig;
    std::unique_ptr<SmMathConfig> mpConfig;
    std::unique_ptr<SmLocalizedSymbolData> mpLocSymbolData;
    std::unique_ptr<SvtSysLocale> mpSysLocale;
    VclPtr<VirtualDevice>    mpVirtualDev;

    static void ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg );

public:
    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START + 0)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    SmModule(SfxObjectFactory* pObjFact);
    virtual ~SmModule();

    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 ) override;

    svtools::ColorConfig &  GetColorConfig();

    SmMathConfig *          GetConfig();
    SmSymbolManager &       GetSymbolManager();

    SmLocalizedSymbolData &   GetLocSymbolData();

    static void GetState(SfxItemSet&);

    const SvtSysLocale& GetSysLocale();

    VirtualDevice &     GetDefaultVirtualDev();

    //virtual methods for options dialog
    virtual SfxItemSet*  CreateItemSet( sal_uInt16 nId ) override;
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet ) override;
    virtual VclPtr<SfxTabPage> CreateTabPage( sal_uInt16 nId, vcl::Window* pParent, const SfxItemSet& rSet ) override;
};

#define SM_MOD() ( *reinterpret_cast<SmModule**>(GetAppData(SHL_SM)) )

#endif // INCLUDED_STARMATH_INC_SMMOD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
